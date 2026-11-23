/*
 * asciiverse/solve.c - finds a solution to an Enigma level, for the
 * catalog preview (asciiverse/record-preview.sh), using Enigma's own
 * engine (engine.c, levelfile.c, memory.c, misc.c), so every move is
 * judged by the real rules: falling arrows, bombs, sacks and all.
 *
 * The search goes from gold to gold. From a position, a breadth-first
 * search over moves (h, j, k, l) finds the nearest positions where one
 * more piece of gold has been taken and the player is still alive. A
 * beam search keeps the most promising of those for the next piece:
 * most gold still reachable (a flood fill through open ground, earth,
 * gold and pushable pieces), spread over the board so the beam does not
 * fill with one corner's near-copies. With all the gold taken, a last
 * search walks to the exit. Prints the moves as one line of h, j, k and
 * l. Enigma's levels are hard: for level 1 this finds 23 of the 61
 * pieces of gold, not a full solution, and with SOLVE_PARTIAL set it
 * prints that best line instead of failing (the preview plays it).
 *
 *   cc -std=gnu89 -O2 -DCURSES_HDR='<ncurses.h>' -I. -o solve \
 *      asciiverse/solve.c engine.c levelfile.c memory.c misc.c
 *   ./solve <level> [beam] [tries] [per area]   (the original set, beside
 *                                                 the program)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "enigma.h"

#define MAXNODES 400000
#define HASHSIZE (1 << 21)

typedef struct {
    gamestate *gs;
    int parent;
    char move;
    char goal;			       /* ends a leg: not searched on from */
} node;

static node *nodes;
static int nnodes;
static int *table;		       /* open addressing: node index + 1 */
static long budget = 60000000;
static int best_gold = -1;	       /* most gold taken on any line tried */
static gamestate *best;
static char *best_moves;	       /* ...and how */	       /* nodes expanded in all, before giving up */

static unsigned long hash_state(gamestate *gs) {
    unsigned long h = 5381;
    int i, n = gs->width * gs->height;
    for (i = 0; i < n; i++)
	h = h * 33 + (unsigned char)gs->leveldata[i];
    return h;
}

static int seen_or_add(gamestate *gs) {
    unsigned long h = hash_state(gs) & (HASHSIZE - 1);
    int n = gs->width * gs->height;
    while (table[h]) {
	if (!memcmp(nodes[table[h]-1].gs->leveldata, gs->leveldata, n))
	    return 1;
	h = (h + 1) & (HASHSIZE - 1);
    }
    table[h] = nnodes + 1;
    return 0;
}

static void reset(void) {
    int i;
    for (i = 0; i < nnodes; i++)
	gamestate_free(nodes[i].gs);
    nnodes = 0;
    memset(table, 0, sizeof(int) * HASHSIZE);
}

/* the moves from node 0 to node i */
static char *path_to(int i) {
    int len = 0, j;
    char *p;
    for (j = i; nodes[j].parent >= 0; j = nodes[j].parent)
	len++;
    p = smalloc(len + 1);
    p[len] = '\0';
    for (j = i; nodes[j].parent >= 0; j = nodes[j].parent)
	p[--len] = nodes[j].move;
    return p;
}

/*
 * Breadth-first from start. Returns up to `want' paths (in out[]) that
 * end where the gold count first goes up (or, with all gold taken,
 * where the level is complete), nearest first.
 */
static int search(gamestate *start, char **out, int want) {
    static const char moves[] = "hjkl";
    int head = 0, found = 0, i;
    int final = start->gold_got >= start->gold_total;

    reset();
    nodes[0].gs = gamestate_copy(start);
    nodes[0].parent = -1;
    nodes[0].goal = 0;
    seen_or_add(nodes[0].gs);
    nnodes = 1;
    while (head < nnodes && found < want && nnodes < MAXNODES - 4) {
	gamestate *gs = nodes[head].gs;
	if (nodes[head].goal) {
	    head++;
	    continue;
	}
	if (--budget < 0)
	    break;
	for (i = 0; i < 4; i++) {
	    gamestate *ng = make_move(gs, moves[i]);
	    if (ng->status == DIED) {
		gamestate_free(ng);
		continue;
	    }
	    if (seen_or_add(ng)) {
		gamestate_free(ng);
		continue;
	    }
	    nodes[nnodes].gs = ng;
	    nodes[nnodes].parent = head;
	    nodes[nnodes].move = moves[i];
	    nodes[nnodes].goal = 0;
	    nnodes++;
	    if ((final && ng->status == COMPLETED) ||
		(!final && ng->gold_got > start->gold_got)) {
		if (found < want)
		    out[found++] = path_to(nnodes - 1);
		nodes[nnodes-1].goal = 1;
	    }
	}
	head++;
    }
    return found;
}

static gamestate *play(gamestate *gs, const char *moves) {
    gamestate *cur = gamestate_copy(gs), *ng;
    for (; *moves; moves++) {
	ng = make_move(cur, *moves);
	gamestate_free(cur);
	cur = ng;
    }
    return cur;
}

/*
 * How much of the gold still lying on the board the player could walk
 * to, by a flood fill through open ground, earth and gold. It ignores
 * everything that moves, so it is only a guide; but a position that has
 * walled gold off scores low.
 */
static int reachable_gold(gamestate *gs) {
    static char seen[64*64];
    static int stack[64*64];
    int w = gs->width, h = gs->height, sp = 0, gold = 0;
    memset(seen, 0, w*h);
    stack[sp++] = gs->player_y * w + gs->player_x;
    seen[stack[0]] = 1;
    while (sp) {
	int p = stack[--sp], x = p % w, y = p / w, d;
	static const int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
	if (gs->leveldata[p] == '$')
	    gold++;
	for (d = 0; d < 4; d++) {
	    int nx = x + dx[d], ny = y + dy[d], q = ny * w + nx;
	    char c;
	    if (nx < 0 || ny < 0 || nx >= w || ny >= h || seen[q])
		continue;
	    c = gs->leveldata[q];
	    if (strchr(" .:$E!=v<>^o8WXYZ", c) && c) {
		seen[q] = 1;
		stack[sp++] = q;
	    }
	}
    }
    return gold;
}

typedef struct {
    gamestate *gs;
    char *moves;
    int score;
} cand;

static int by_score(const void *a, const void *b) {
    const cand *x = a, *y = b;
    if (x->score != y->score)
	return y->score - x->score;
    return (int)strlen(x->moves) - (int)strlen(y->moves);
}

/*
 * Beam search over the gold: each layer holds up to `beam' positions
 * with one more piece of gold than the layer before, the most promising
 * first (most of the remaining gold still reachable, then fewest
 * moves). Returns the moves of a full solution, or NULL.
 */
static char *beam_solve(gamestate *start, int beam, int tries, int per_area) {
    cand *layer = smalloc(sizeof(cand)), *next;
    int nlayer = 1, nnext, i, j, k;
    char *legs[64];

    layer[0].gs = gamestate_copy(start);
    layer[0].moves = dupstr("");
    while (nlayer) {
	next = smalloc(sizeof(cand) * nlayer * tries);
	nnext = 0;
	for (i = 0; i < nlayer; i++) {
	    gamestate *gs = layer[i].gs;
	    int final = gs->gold_got >= gs->gold_total;
	    int n = search(gs, legs, final ? 1 : tries);
	    if (gs->gold_got > best_gold) {
		best_gold = gs->gold_got;
		if (best)
		    gamestate_free(best);
		best = gamestate_copy(gs);
		sfree(best_moves);
		best_moves = dupstr(layer[i].moves);
	    }
	    for (j = 0; j < n; j++) {
		gamestate *ng = play(gs, legs[j]);
		char *m = smalloc(strlen(layer[i].moves) + strlen(legs[j]) + 1);
		strcpy(m, layer[i].moves);
		strcat(m, legs[j]);
		sfree(legs[j]);
		if (ng->status == COMPLETED) {
		    for (k = 0; k < nlayer; k++) {
			gamestate_free(layer[k].gs);
			sfree(layer[k].moves);
		    }
		    for (k = 0; k < nnext; k++) {
			gamestate_free(next[k].gs);
			sfree(next[k].moves);
		    }
		    gamestate_free(ng);
		    return m;
		}
		/* the same board reached twice: keep the shorter way */
		for (k = 0; k < nnext; k++)
		    if (!memcmp(next[k].gs->leveldata, ng->leveldata,
				ng->width * ng->height))
			break;
		if (k < nnext) {
		    if (strlen(m) < strlen(next[k].moves)) {
			sfree(next[k].moves);
			next[k].moves = m;
		    } else
			sfree(m);
		    gamestate_free(ng);
		    continue;
		}
		next[nnext].gs = ng;
		next[nnext].moves = m;
		next[nnext].score = reachable_gold(ng) * 4 + ng->gold_got * 4;
		nnext++;
	    }
	}
	for (k = 0; k < nlayer; k++) {
	    gamestate_free(layer[k].gs);
	    sfree(layer[k].moves);
	}
	sfree(layer);
	qsort(next, nnext, sizeof(cand), by_score);
	/*
	 * Keep the beam spread out: at most `per_area' positions with the
	 * player in the same 6x5 area of the board, so it does not fill
	 * up with one corner's near-copies.
	 */
	{
	    int count[64], kept = 0;
	    memset(count, 0, sizeof(count));
	    for (k = 0; k < nnext; k++) {
		int a = (next[k].gs->player_y / 5) * 8 + next[k].gs->player_x / 6;
		if (kept < beam && count[a & 63] < per_area) {
		    count[a & 63]++;
		    next[kept++] = next[k];
		} else {
		    gamestate_free(next[k].gs);
		    sfree(next[k].moves);
		}
	    }
	    nnext = kept;
	}
	layer = next;
	nlayer = nnext;
	if (budget < 0)
	    break;
    }
    return NULL;
}

int main(int argc, char **argv) {
    levelset *set;
    gamestate *gs;
    char *sol;
    int lev = argc > 1 ? atoi(argv[1]) : 1;
    int beam = argc > 2 ? atoi(argv[2]) : 300;

    find_dirs();
    if (setjmp(fatal_error_jmp_buf)) {
	fprintf(stderr, "solve: %s\n", fatal_error_string);
	return 2;
    }
    set = levelset_load(DEFAULTSET);
    if (lev < 1 || lev > set->nlevels) {
	fprintf(stderr, "solve: no level %d\n", lev);
	return 2;
    }
    nodes = smalloc(sizeof(node) * MAXNODES);
    table = smalloc(sizeof(int) * HASHSIZE);
    nnodes = 0;
    memset(table, 0, sizeof(int) * HASHSIZE);
    gs = init_game(set->levels[lev-1]);
    gs->levnum = lev;
    sol = beam_solve(gs, beam, argc > 3 ? atoi(argv[3]) : 40,
		     argc > 4 ? atoi(argv[4]) : 20);
    if (!sol) {
	fprintf(stderr, "solve: no solution found (best: %d of %d gold)\n",
		best_gold, gs->gold_total);
	if (getenv("SOLVE_SHOW") && best) {
	    int y;
	    for (y = 0; y < best->height; y++)
		fprintf(stderr, "%.*s\n", best->width,
			best->leveldata + y * best->width);
	}
	if (getenv("SOLVE_PARTIAL") && best_moves) {
	    printf("%s\n", best_moves);   /* the best line found, alive */
	    return 0;
	}
	return 1;
    }
    printf("%s\n", sol);
    return 0;
}
