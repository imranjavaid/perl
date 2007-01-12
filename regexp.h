/*    regexp.h
 *
 *    Copyright (C) 1993, 1994, 1996, 1997, 1999, 2000, 2001, 2003,
 *    2005, 2006 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#ifndef PLUGGABLE_RE_EXTENSION
/* we don't want to include this stuff if we are inside of
   an external regex engine based on the core one - like re 'debug'*/

struct regnode {
    U8	flags;
    U8  type;
    U16 next_off;
};

typedef struct regnode regnode;

struct reg_substr_data;

struct reg_data;

struct regexp_engine;

struct reg_substr_datum {
    I32 min_offset;
    I32 max_offset;
    SV *substr;		/* non-utf8 variant */
    SV *utf8_substr;	/* utf8 variant */
    I32 end_shift;
};
struct reg_substr_data {
    struct reg_substr_datum data[3];	/* Actual array */
};

#ifdef PERL_OLD_COPY_ON_WRITE
#define SV_SAVED_COPY   SV *saved_copy; /* If non-NULL, SV which is COW from original */
#else
#define SV_SAVED_COPY
#endif
/* this is ordered such that the most commonly used 
   fields are at the start of the struct */
typedef struct regexp {
        /* what engine created this regexp? */
	const struct regexp_engine* engine; 
	
	/* Information about the match that the perl core uses to manage things */
	U32 extflags;           /* Flags used both externally and internally */
	I32 minlen;		/* mininum possible length of string to match */
	I32 minlenret;		/* mininum possible length of $& */
	U32 gofs;               /* chars left of pos that we search from */
	struct reg_substr_data *substrs; /* substring data about strings that must appear
                                   in the final match, used for optimisations */
	U32 nparens;		/* number of capture buffers */

        /* private engine specific data */
	U32 intflags;		/* Engine Specific Internal flags */
	void *pprivate;         /* Data private to the regex engine which 
                                   created this object. */
        
        /* Data about the last/current match. These are modified during matching*/
        U32 lastparen;		/* last open paren matched */
	U32 lastcloseparen;	/* last close paren matched */
        I32 *startp;            /* Array of offsets from start of string (@-) */
	I32 *endp;              /* Array of offsets from start of string (@+) */
	char *subbeg;		/* saved or original string 
				   so \digit works forever. */
	I32 sublen;		/* Length of string pointed by subbeg */
	SV_SAVED_COPY           /* If non-NULL, SV which is COW from original */
        
        
        /* Information about the match that isn't often used */
	char *precomp;		/* pre-compilation regular expression */
	I32 prelen;		/* length of precomp */
	I32 seen_evals;         /* number of eval groups in the pattern - for security checks */ 
        HV *paren_names;	/* Optional hash of paren names */
        
        /* Refcount of this regexp */
	I32 refcnt;             /* Refcount of this regexp */
} regexp;

/* used for high speed searches */
typedef struct re_scream_pos_data_s
{
    char **scream_olds;		/* match pos */
    I32 *scream_pos;		/* Internal iterator of scream. */
} re_scream_pos_data;

/* regexp_engine structure. This is the dispatch table for regexes.
 * Any regex engine implementation must be able to build one of these.
 */
typedef struct regexp_engine {
    regexp* (*comp) (pTHX_ char* exp, char* xend, PMOP* pm);
    I32	    (*exec) (pTHX_ regexp* prog, char* stringarg, char* strend,
			    char* strbeg, I32 minend, SV* screamer,
			    void* data, U32 flags);
    char*   (*intuit) (pTHX_ regexp *prog, SV *sv, char *strpos,
			    char *strend, U32 flags,
			    struct re_scream_pos_data_s *data);
    SV*	    (*checkstr) (pTHX_ regexp *prog);
    void    (*free) (pTHX_ struct regexp* r);
#ifdef USE_ITHREADS
    void* (*dupe) (pTHX_ const regexp *r, CLONE_PARAMS *param);
#endif    
} regexp_engine;

/* Flags stored in regexp->extflags 
 * These are used by code external to the regexp engine
 *
 * Note that flags starting with RXf_PMf_ have exact equivalents
 * stored in op_pmflags and which are defined in op.h, they are defined
 * numerically here only for clarity.
 */

/* Anchor and GPOS related stuff */
#define RXf_ANCH_BOL    	0x00000001
#define RXf_ANCH_MBOL   	0x00000002
#define RXf_ANCH_SBOL   	0x00000004
#define RXf_ANCH_GPOS   	0x00000008
#define RXf_GPOS_SEEN   	0x00000010
#define RXf_GPOS_FLOAT  	0x00000020
/* two bits here */
#define RXf_ANCH        	(RXf_ANCH_BOL|RXf_ANCH_MBOL|RXf_ANCH_GPOS|RXf_ANCH_SBOL)
#define RXf_GPOS_CHECK          (RXf_GPOS_SEEN|RXf_ANCH_GPOS)
#define RXf_ANCH_SINGLE         (RXf_ANCH_SBOL|RXf_ANCH_GPOS)

/* Flags indicating special patterns */
#define RXf_START_ONLY		0x00000200 /* Pattern is /^/ */
#define RXf_WHITE		0x00000400 /* Pattern is /\s+/ */

/* 0xF800 of extflags is used by (RXf_)PMf_COMPILETIME */
#define RXf_PMf_LOCALE  	0x00000800 /* use locale */
#define RXf_PMf_MULTILINE	0x00001000 /* /m         */
#define RXf_PMf_SINGLELINE	0x00002000 /* /s         */
#define RXf_PMf_FOLD    	0x00004000 /* /i         */
#define RXf_PMf_EXTENDED	0x00008000 /* /x         */
#define RXf_PMf_KEEPCOPY	0x00010000 /* /k         */
/* these flags are transfered from the PMOP->op_pmflags member during compilation */
#define RXf_PMf_STD_PMMOD	(RXf_PMf_MULTILINE|RXf_PMf_SINGLELINE|RXf_PMf_FOLD|RXf_PMf_EXTENDED)
#define RXf_PMf_COMPILETIME	(RXf_PMf_MULTILINE|RXf_PMf_SINGLELINE|RXf_PMf_LOCALE|RXf_PMf_FOLD|RXf_PMf_EXTENDED|RXf_PMf_KEEPCOPY)

#define CASE_STD_PMMOD_FLAGS_PARSE_SET(pmfl)              \
        case 'i': *(pmfl) |= RXf_PMf_FOLD;       break;   \
        case 'm': *(pmfl) |= RXf_PMf_MULTILINE;  break;   \
        case 's': *(pmfl) |= RXf_PMf_SINGLELINE; break;   \
        case 'x': *(pmfl) |= RXf_PMf_EXTENDED;   break

/* What we have seen */
#define RXf_LOOKBEHIND_SEEN	0x00020000
#define RXf_EVAL_SEEN   	0x00040000
#define RXf_CANY_SEEN   	0x00080000

/* Special */
#define RXf_NOSCAN      	0x00100000
#define RXf_CHECK_ALL   	0x00200000

/* UTF8 related */
#define RXf_UTF8        	0x00400000
#define RXf_MATCH_UTF8  	0x00800000

/* Intuit related */
#define RXf_USE_INTUIT_NOML	0x01000000
#define RXf_USE_INTUIT_ML	0x02000000
#define RXf_INTUIT_TAIL 	0x04000000
/* one bit here */
#define RXf_USE_INTUIT		(RXf_USE_INTUIT_NOML|RXf_USE_INTUIT_ML)

/* Copy and tainted info */
#define RXf_COPY_DONE   	0x10000000
#define RXf_TAINTED_SEEN	0x20000000
/* two bits here  */


#define RX_HAS_CUTGROUP(prog) ((prog)->intflags & PREGf_CUTGROUP_SEEN)
#define RX_MATCH_TAINTED(prog)	((prog)->extflags & RXf_TAINTED_SEEN)
#define RX_MATCH_TAINTED_on(prog) ((prog)->extflags |= RXf_TAINTED_SEEN)
#define RX_MATCH_TAINTED_off(prog) ((prog)->extflags &= ~RXf_TAINTED_SEEN)
#define RX_MATCH_TAINTED_set(prog, t) ((t) \
				       ? RX_MATCH_TAINTED_on(prog) \
				       : RX_MATCH_TAINTED_off(prog))

#define RX_MATCH_COPIED(prog)		((prog)->extflags & RXf_COPY_DONE)
#define RX_MATCH_COPIED_on(prog)	((prog)->extflags |= RXf_COPY_DONE)
#define RX_MATCH_COPIED_off(prog)	((prog)->extflags &= ~RXf_COPY_DONE)
#define RX_MATCH_COPIED_set(prog,t)	((t) \
					 ? RX_MATCH_COPIED_on(prog) \
					 : RX_MATCH_COPIED_off(prog))

#endif /* PLUGGABLE_RE_EXTENSION */

/* Stuff that needs to be included in the plugable extension goes below here */

#ifdef PERL_OLD_COPY_ON_WRITE
#define RX_MATCH_COPY_FREE(rx) \
	STMT_START {if (rx->saved_copy) { \
	    SV_CHECK_THINKFIRST_COW_DROP(rx->saved_copy); \
	} \
	if (RX_MATCH_COPIED(rx)) { \
	    Safefree(rx->subbeg); \
	    RX_MATCH_COPIED_off(rx); \
	}} STMT_END
#else
#define RX_MATCH_COPY_FREE(rx) \
	STMT_START {if (RX_MATCH_COPIED(rx)) { \
	    Safefree(rx->subbeg); \
	    RX_MATCH_COPIED_off(rx); \
	}} STMT_END
#endif

#define RX_MATCH_UTF8(prog)		((prog)->extflags & RXf_MATCH_UTF8)
#define RX_MATCH_UTF8_on(prog)		((prog)->extflags |= RXf_MATCH_UTF8)
#define RX_MATCH_UTF8_off(prog)		((prog)->extflags &= ~RXf_MATCH_UTF8)
#define RX_MATCH_UTF8_set(prog, t)	((t) \
			? (RX_MATCH_UTF8_on(prog), (PL_reg_match_utf8 = 1)) \
			: (RX_MATCH_UTF8_off(prog), (PL_reg_match_utf8 = 0)))
    
#define REXEC_COPY_STR	0x01		/* Need to copy the string. */
#define REXEC_CHECKED	0x02		/* check_substr already checked. */
#define REXEC_SCREAM	0x04		/* use scream table. */
#define REXEC_IGNOREPOS	0x08		/* \G matches at start. */
#define REXEC_NOT_FIRST	0x10		/* This is another iteration of //g. */

#define ReREFCNT_inc(re) ((void)(re && re->refcnt++), re)
#define ReREFCNT_dec(re) CALLREGFREE(re)

#define FBMcf_TAIL_DOLLAR	1
#define FBMcf_TAIL_DOLLARM	2
#define FBMcf_TAIL_Z		4
#define FBMcf_TAIL_z		8
#define FBMcf_TAIL		(FBMcf_TAIL_DOLLAR|FBMcf_TAIL_DOLLARM|FBMcf_TAIL_Z|FBMcf_TAIL_z)

#define FBMrf_MULTILINE	1

/* an accepting state/position*/
struct _reg_trie_accepted {
    U8   *endpos;
    U16  wordnum;
};
typedef struct _reg_trie_accepted reg_trie_accepted;

/* some basic information about the current match that is created by
 * Perl_regexec_flags and then passed to regtry(), regmatch() etc */

typedef struct {
    regexp *prog;
    char *bol;
    char *till;
    SV *sv;
    char *ganch;
    char *cutpoint;
} regmatch_info;
 

/* structures for holding and saving the state maintained by regmatch() */

#define MAX_RECURSE_EVAL_NOCHANGE_DEPTH 50

typedef I32 CHECKPOINT;

typedef struct regmatch_state {
    int resume_state;		/* where to jump to on return */
    char *locinput;		/* where to backtrack in string on failure */

    union {

	/* this is a fake union member that matches the first element
	 * of each member that needs to store positive backtrack
	 * information */
	struct {
	    struct regmatch_state *prev_yes_state;
	} yes;

	struct {
	    /* this first element must match u.yes */
	    struct regmatch_state *prev_yes_state;
	    reg_trie_accepted *accept_buff;
	    U32		accepted; /* how many accepting states we have seen */
	    U16         *jump;  /* positive offsets from me */
	    regnode	*B;	/* node following the trie */
	    regnode	*me;	/* Which node am I - needed for jump tries*/
	} trie;

	struct {
	    /* this first element must match u.yes */
	    struct regmatch_state *prev_yes_state;
	    struct regmatch_state *prev_eval;
	    struct regmatch_state *prev_curlyx;
	    regexp	*prev_rex;
	    U32		toggle_reg_flags; /* what bits in PL_reg_flags to
					    flip when transitioning between
					    inner and outer rexen */
	    CHECKPOINT	cp;	/* remember current savestack indexes */
	    CHECKPOINT	lastcp;
	    regnode	*B;	/* the node following us  */
	    U32        close_paren; /* which close bracket is our end */
	} eval;

	struct {
	    /* this first element must match u.yes */
	    struct regmatch_state *prev_yes_state;
	    struct regmatch_state *prev_curlyx; /* previous cur_curlyx */
	    CHECKPOINT	cp;	/* remember current savestack index */
	    bool	minmod;
	    int		parenfloor;/* how far back to strip paren data */
	    int		min;	/* the minimal number of A's to match */
	    int		max;	/* the maximal number of A's to match */
	    regnode	*A, *B;	/* the nodes corresponding to /A*B/  */

	    /* these two are modified by WHILEM */
	    int		count;	/* how many instances of A we've matched */
	    char	*lastloc;/* where previous A matched (0-len detect) */
	} curlyx;

	struct {
	    /* this first element must match u.yes */
	    struct regmatch_state *prev_yes_state;
	    struct regmatch_state *save_curlyx;
	    CHECKPOINT	cp;	/* remember current savestack indexes */
	    CHECKPOINT	lastcp;
	    char	*save_lastloc;	/* previous curlyx.lastloc */
	    I32		cache_offset;
	    I32		cache_mask;
	} whilem;

	struct {
	    /* this first element must match u.yes */
	    struct regmatch_state *prev_yes_state;
	    U32 lastparen;
	    regnode *next_branch; /* next branch node */
	    CHECKPOINT cp;
	} branch;

	struct {
	    /* this first element must match u.yes */
	    struct regmatch_state *prev_yes_state;
	    I32 c1, c2;		/* case fold search */
	    CHECKPOINT cp;
	    I32 alen;		/* length of first-matched A string */
	    I32 count;
	    bool minmod;
	    regnode *A, *B;	/* the nodes corresponding to /A*B/  */
	    regnode *me;	/* the curlym node */
	} curlym;

	struct {
	    U32 paren;
	    CHECKPOINT cp;
	    I32 c1, c2;		/* case fold search */
	    char *maxpos;	/* highest possible point in string to match */
	    char *oldloc;	/* the previous locinput */
	    int count;
	    int min, max;	/* {m,n} */
	    regnode *A, *B;	/* the nodes corresponding to /A*B/  */
	} curly; /* and CURLYN/PLUS/STAR */

	struct {
	    /* this first element must match u.yes */
	    struct regmatch_state *prev_yes_state;
	    I32 wanted;
	    I32 logical;	/* saved copy of 'logical' var */
	    regnode  *me; /* the IFMATCH/SUSPEND/UNLESSM node  */
	} ifmatch; /* and SUSPEND/UNLESSM */
	
	struct {
	    /* this first element must match u.yes */
	    struct regmatch_state *prev_yes_state;
	    struct regmatch_state *prev_mark;
	    SV* mark_name;
	    char *mark_loc;
	} mark;
	
	struct {
	    int val;
	} keeper;
    } u;
} regmatch_state;

/* how many regmatch_state structs to allocate as a single slab.
 * We do it in 4K blocks for efficiency. The "3" is 2 for the next/prev
 * pointers, plus 1 for any mythical malloc overhead. */
 
#define PERL_REGMATCH_SLAB_SLOTS \
    ((4096 - 3 * sizeof (void*)) / sizeof(regmatch_state))

typedef struct regmatch_slab {
    regmatch_state states[PERL_REGMATCH_SLAB_SLOTS];
    struct regmatch_slab *prev, *next;
} regmatch_slab;

#define PL_reg_flags		PL_reg_state.re_state_reg_flags
#define PL_bostr		PL_reg_state.re_state_bostr
#define PL_reginput		PL_reg_state.re_state_reginput
#define PL_regeol		PL_reg_state.re_state_regeol
#define PL_regstartp		PL_reg_state.re_state_regstartp
#define PL_regendp		PL_reg_state.re_state_regendp
#define PL_reglastparen		PL_reg_state.re_state_reglastparen
#define PL_reglastcloseparen	PL_reg_state.re_state_reglastcloseparen
#define PL_reg_start_tmp	PL_reg_state.re_state_reg_start_tmp
#define PL_reg_start_tmpl	PL_reg_state.re_state_reg_start_tmpl
#define PL_reg_eval_set		PL_reg_state.re_state_reg_eval_set
#define PL_reg_match_utf8	PL_reg_state.re_state_reg_match_utf8
#define PL_reg_magic		PL_reg_state.re_state_reg_magic
#define PL_reg_oldpos		PL_reg_state.re_state_reg_oldpos
#define PL_reg_oldcurpm		PL_reg_state.re_state_reg_oldcurpm
#define PL_reg_curpm		PL_reg_state.re_state_reg_curpm
#define PL_reg_oldsaved		PL_reg_state.re_state_reg_oldsaved
#define PL_reg_oldsavedlen	PL_reg_state.re_state_reg_oldsavedlen
#define PL_reg_maxiter		PL_reg_state.re_state_reg_maxiter
#define PL_reg_leftiter		PL_reg_state.re_state_reg_leftiter
#define PL_reg_poscache		PL_reg_state.re_state_reg_poscache
#define PL_reg_poscache_size	PL_reg_state.re_state_reg_poscache_size
#define PL_regsize		PL_reg_state.re_state_regsize
#define PL_reg_starttry		PL_reg_state.re_state_reg_starttry
#define PL_nrs			PL_reg_state.re_state_nrs

struct re_save_state {
    U32 re_state_reg_flags;		/* from regexec.c */
    char *re_state_bostr;
    char *re_state_reginput;		/* String-input pointer. */
    char *re_state_regeol;		/* End of input, for $ check. */
    I32 *re_state_regstartp;		/* Pointer to startp array. */
    I32 *re_state_regendp;		/* Ditto for endp. */
    U32 *re_state_reglastparen;		/* Similarly for lastparen. */
    U32 *re_state_reglastcloseparen;	/* Similarly for lastcloseparen. */
    char **re_state_reg_start_tmp;	/* from regexec.c */
    U32 re_state_reg_start_tmpl;	/* from regexec.c */
    I32 re_state_reg_eval_set;		/* from regexec.c */
    bool re_state_reg_match_utf8;	/* from regexec.c */
    MAGIC *re_state_reg_magic;		/* from regexec.c */
    I32 re_state_reg_oldpos;		/* from regexec.c */
    PMOP *re_state_reg_oldcurpm;	/* from regexec.c */
    PMOP *re_state_reg_curpm;		/* from regexec.c */
    char *re_state_reg_oldsaved;	/* old saved substr during match */
    STRLEN re_state_reg_oldsavedlen;	/* old length of saved substr during match */
    I32 re_state_reg_maxiter;		/* max wait until caching pos */
    I32 re_state_reg_leftiter;		/* wait until caching pos */
    char *re_state_reg_poscache;	/* cache of pos of WHILEM */
    STRLEN re_state_reg_poscache_size;	/* size of pos cache of WHILEM */
    U32 re_state_regsize;		/* from regexec.c */
    char *re_state_reg_starttry;	/* from regexec.c */
#ifdef PERL_OLD_COPY_ON_WRITE
    SV *re_state_nrs;			/* was placeholder: unused since 5.8.0 (5.7.2 patch #12027 for bug ID 20010815.012). Used to save rx->saved_copy */
#endif
};

#define SAVESTACK_ALLOC_FOR_RE_SAVE_STATE \
	(1 + ((sizeof(struct re_save_state) - 1) / sizeof(*PL_savestack)))

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
