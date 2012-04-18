/*  Boolector: Satisfiablity Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2010 Robert Daniel Brummayer.
 *  Copyright (C) 2010-2012 Armin Biere.
 *
 *  All rights reserved.
 *
 *  This file is part of Boolector.
 *  See COPYING for more information on using this software.
 */

#ifdef BTOR_USE_PICOSAT
#include "../picosat/picosat.h"
#endif

#ifdef BTOR_USE_LINGELING
#include "../lingeling/lglib.h"
#endif

#ifdef BTOR_USE_MINISAT
#include "btorminisat.h"
#endif

#include "btorexit.h"
#include "btorsat.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>

/*------------------------------------------------------------------------*/

#define BTOR_ABORT_SAT(cond, msg)                   \
  do                                                \
  {                                                 \
    if (cond)                                       \
    {                                               \
      printf ("[btorsat] %s: %s\n", __func__, msg); \
      fflush (stdout);                              \
      exit (BTOR_ERR_EXIT);                         \
    }                                               \
  } while (0)

/*------------------------------------------------------------------------*/

void
btor_msg_sat (BtorSATMgr *smgr, int level, const char *fmt, ...)
{
  va_list ap;
  if (smgr->verbosity < level) return;
  assert (fmt != NULL);
  fprintf (stdout, "[btorsat] ");
  va_start (ap, fmt);
  vfprintf (stdout, fmt, ap);
  va_end (ap);
  putc ('\n', stdout);
  fflush (stdout);
}

/*------------------------------------------------------------------------*/

#if defined(BTOR_USE_LINGELING)
int btor_enable_lingeling_sat (BtorSATMgr *,
                               const char *optstr,
                               int nofork,
                               int nobrutefork);
#define btor_enable_default_sat(SMGR)            \
  do                                             \
  {                                              \
    btor_enable_lingeling_sat ((SMGR), 0, 0, 0); \
  } while (0)
#elif defined(BTOR_USE_PICOSAT)
void btor_enable_picosat_sat (BtorSATMgr *);
#define btor_enable_default_sat btor_enable_picosat_sat
#elif defined(BTOR_USE_MINISAT)
void btor_enable_minisat_sat (BtorSATMgr *);
#define btor_enable_default_sat btor_enable_minisat_sat
#else
#error "no usable SAT solver configured"
#endif

/*------------------------------------------------------------------------*/

BtorSATMgr *
btor_new_sat_mgr (BtorMemMgr *mm)
{
  BtorSATMgr *smgr;

  assert (mm != NULL);

  BTOR_NEW (mm, smgr);

  smgr->verbosity   = 0;
  smgr->mm          = mm;
  smgr->satcalls    = 0;
  smgr->initialized = 0;
  smgr->clauses = smgr->maxvar = 0;
  smgr->output                 = stdout;

  btor_enable_default_sat (smgr);

  return smgr;
}

void
btor_set_verbosity_sat_mgr (BtorSATMgr *smgr, int verbosity)
{
  assert (verbosity >= -1 && verbosity <= 3);
  smgr->verbosity = verbosity;
}

int
btor_is_initialized_sat (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  return smgr->initialized;
}

int
btor_next_cnf_id_sat_mgr (BtorSATMgr *smgr)
{
  int result;
  assert (smgr);
  assert (smgr->initialized);
  result = smgr->api.inc_max_var (smgr);
  if (abs (result) > smgr->maxvar) smgr->maxvar = abs (result);
  BTOR_ABORT_SAT (result <= 0, "CNF id overflow");
  if (smgr->verbosity > 2 && !(result % 100000))
    btor_msg_sat (smgr, 2, "reached CNF id %d", result);
  return result;
}

void
btor_release_cnf_id_sat_mgr (BtorSATMgr *smgr, int lit)
{
  assert (smgr);
  if (!smgr->initialized) return;
  assert (abs (lit) <= smgr->maxvar);
  if (abs (lit) == smgr->true_lit) return;
  if (smgr->api.melt) smgr->api.melt (smgr, lit);
}

int
btor_get_last_cnf_id_sat_mgr (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  (void) smgr;
  return smgr->api.variables (smgr);
}

void
btor_delete_sat_mgr (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  /* if SAT is still initialized, then
   * reset_sat has not been called
   */
  if (smgr->initialized) btor_reset_sat (smgr);
  BTOR_DELETE (smgr->mm, smgr);
}

/*------------------------------------------------------------------------*/

void
btor_init_sat (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  assert (!smgr->initialized);

  smgr->solver      = smgr->api.init (smgr);
  smgr->initialized = 1;
  smgr->true_lit    = btor_next_cnf_id_sat_mgr (smgr);
  btor_add_sat (smgr, smgr->true_lit);
  btor_add_sat (smgr, 0);
}

void
btor_set_output_sat (BtorSATMgr *smgr, FILE *output)
{
  char *prefix, *q;
  const char *p;

  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (output != NULL);
  (void) smgr;
  smgr->api.set_output (smgr, output);
  smgr->output = output;

  prefix = btor_malloc (smgr->mm, strlen (smgr->name) + 4);
  sprintf (prefix, "[%s] ", smgr->name);
  q = prefix + 1;
  for (p = smgr->name; *p; p++) *q++ = tolower (*p);
  smgr->api.set_prefix (smgr, prefix);
  btor_free (smgr->mm, prefix, strlen (smgr->name) + 4);
}

void
btor_enable_verbosity_sat (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  (void) smgr;
  smgr->api.enable_verbosity (smgr);
}

void
btor_print_stats_sat (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  if (!smgr->initialized) return;
  smgr->api.stats (smgr);
}

void
btor_add_sat (BtorSATMgr *smgr, int lit)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  if (!lit) smgr->clauses++;
  (void) smgr->api.add (smgr, lit);
}

int
btor_sat_sat (BtorSATMgr *smgr, int limit)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  btor_msg_sat (smgr, 2, "calling SAT solver %s", smgr->name);
  smgr->satcalls++;
  return smgr->api.sat (smgr, limit);
}

int
btor_deref_sat (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  return smgr->api.deref (smgr, lit);
}

int
btor_repr_sat (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  return smgr->api.repr (smgr, lit);
}

void
btor_reset_sat (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  btor_msg_sat (smgr, 2, "resetting %s", smgr->name);
  smgr->api.reset (smgr);
  smgr->solver      = 0;
  smgr->initialized = 0;
}

int
btor_fixed_sat (BtorSATMgr *smgr, int lit)
{
  int res;
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  res = smgr->api.fixed (smgr, lit);
  return res;
}

/*------------------------------------------------------------------------*/

void
btor_assume_sat (BtorSATMgr *smgr, int lit)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  smgr->api.assume (smgr, lit);
}

int
btor_failed_sat (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  return smgr->api.failed (smgr, lit);
}

int
btor_inconsistent_sat (BtorSATMgr *smgr)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  return smgr->api.inconsistent (smgr);
}

int
btor_changed_sat (BtorSATMgr *smgr)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  return smgr->api.changed (smgr);
}

/*------------------------------------------------------------------------*/
#ifdef BTOR_USE_PICOSAT

static void *
btor_picosat_init (BtorSATMgr *smgr)
{
  btor_msg_sat (smgr, 1, "PicoSAT Version %s", picosat_version ());

  picosat_set_new (smgr->mm, (void *(*) (void *, size_t)) btor_malloc);
  picosat_set_delete (smgr->mm, (void (*) (void *, void *, size_t)) btor_free);
  picosat_set_resize (
      smgr->mm, (void *(*) (void *, void *, size_t, size_t)) btor_realloc);

  picosat_init ();
  picosat_set_global_default_phase (0);

  return 0;
}

static void
btor_picosat_add (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  (void) picosat_add (lit);
}

static int
btor_picosat_sat (BtorSATMgr *smgr, int limit)
{
  (void) smgr;
  return picosat_sat (limit);
}

static int
btor_picosat_changed (BtorSATMgr *smgr)
{
  (void) smgr;
  return picosat_changed ();
}

static int
btor_picosat_deref (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  return picosat_deref (lit);
}

static int
btor_picosat_repr (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  return lit;
}

static void
btor_picosat_reset (BtorSATMgr *smgr)
{
  (void) smgr;
  picosat_reset ();
}

static void
btor_picosat_set_output (BtorSATMgr *smgr, FILE *output)
{
  (void) smgr;
  picosat_set_output (output);
}

static void
btor_picosat_set_prefix (BtorSATMgr *smgr, const char *prefix)
{
  (void) smgr;
  picosat_set_prefix (prefix);
}

static void
btor_picosat_enable_verbosity (BtorSATMgr *smgr)
{
  (void) smgr;
  picosat_set_verbosity (1);
}

static int
btor_picosat_inc_max_var (BtorSATMgr *smgr)
{
  (void) smgr;
  return picosat_inc_max_var ();
}

static int
btor_picosat_variables (BtorSATMgr *smgr)
{
  (void) smgr;
  return picosat_variables ();
}

static void
btor_picosat_stats (BtorSATMgr *smgr)
{
  (void) smgr;
  picosat_stats ();
}

static int
btor_picosat_fixed (BtorSATMgr *smgr, int lit)
{
  int res;
  (void) smgr;
  res = picosat_deref_toplevel (lit);
  return res;
}

/*------------------------------------------------------------------------*/

static void
btor_picosat_assume (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  (void) picosat_assume (lit);
}

static int
btor_picosat_failed (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  return picosat_failed_assumption (lit);
}

static int
btor_picosat_inconsistent (BtorSATMgr *smgr)
{
  (void) smgr;
  return picosat_inconsistent ();
}

/*------------------------------------------------------------------------*/

void
btor_enable_picosat_sat (BtorSATMgr *smgr)
{
  assert (smgr != NULL);

  BTOR_ABORT_SAT (smgr->initialized,
                  "'btor_init_sat' called before 'btor_enable_picosat_sat'");

  smgr->name   = "PicoSAT";
  smgr->optstr = 0;

  smgr->api.add              = btor_picosat_add;
  smgr->api.assume           = btor_picosat_assume;
  smgr->api.changed          = btor_picosat_changed;
  smgr->api.deref            = btor_picosat_deref;
  smgr->api.enable_verbosity = btor_picosat_enable_verbosity;
  smgr->api.failed           = btor_picosat_failed;
  smgr->api.fixed            = btor_picosat_fixed;
  smgr->api.inc_max_var      = btor_picosat_inc_max_var;
  smgr->api.inconsistent     = btor_picosat_inconsistent;
  smgr->api.init             = btor_picosat_init;
  smgr->api.melt             = 0;
  smgr->api.repr             = btor_picosat_repr;
  smgr->api.reset            = btor_picosat_reset;
  smgr->api.sat              = btor_picosat_sat;
  smgr->api.set_output       = btor_picosat_set_output;
  smgr->api.set_prefix       = btor_picosat_set_prefix;
  smgr->api.stats            = btor_picosat_stats;
  smgr->api.variables        = btor_picosat_variables;

  btor_msg_sat (
      smgr, 1, "PicoSAT allows both incremental and non-incremental mode");
}

#endif
/*------------------------------------------------------------------------*/
#ifdef BTOR_USE_LINGELING

typedef struct BtorLGL BtorLGL;

struct BtorLGL
{
  LGL *lgl;
  int nforked, nbforked;
};

static int
btor_passdown_lingeling_options (BtorSATMgr *smgr,
                                 const char *optstr,
                                 LGL *external_lgl)
{
  char *str, *p, *next, *eq, *opt, *val;
  LGL *lgl = external_lgl ? external_lgl : 0;
  int len, valid, res = 1;

  assert (optstr);
  len = strlen (optstr);

  BTOR_NEWN (smgr->mm, str, len + 1);
  strcpy (str, optstr);

  res = 1;

  for (p = str; *p; p = next)
  {
    if (*p == ',')
      next = p + 1;
    else
    {
      opt = p;
      while (*p != ',' && *p) p++;

      if (*p)
      {
        assert (*p == ',');
        *p   = 0;
        next = p + 1;
      }
      else
        next = p;

      val = eq = 0;

      if (!isalpha (*opt))
        valid = 0;
      else
      {
        for (p = opt + 1; isalnum (*p); p++)
          ;

        if (*p == '=')
        {
          *(eq = p++) = 0;
          val         = p;
          if (*p == '-') p++;
          if (isdigit (*p))
          {
            while (isdigit (*p)) p++;

            valid = !*p;
          }
          else
            valid = 0;
        }
        else
          valid = 0;
      }

      if (valid)
      {
        if (!lgl)
        {
          assert (!external_lgl);
          lgl = lglinit ();
        }

        if (lglhasopt (lgl, opt))
        {
          if (external_lgl && val)
          {
            assert (lgl == external_lgl);
            btor_msg_sat (
                smgr, 2, "setting Lingeling option --%s=%s", opt, val);
            lglsetopt (lgl, opt, atoi (val));
          }
        }
        else
          valid = 0;
      }

      if (!valid) res = 0;
      if (valid || external_lgl) continue;

      if (eq) *eq = '=';
      btor_msg_sat (smgr,
                    0,
                    "*** can not pass down to Lingeling invalid option '%s'",
                    optstr);
    }
  }

  BTOR_DELETEN (smgr->mm, str, len + 1);
  if (lgl && !external_lgl) lglrelease (lgl);

  return res;
}

static void *
btor_lingeling_init (BtorSATMgr *smgr)
{
  BtorLGL *res;
  if (smgr->verbosity >= 1)
  {
    lglbnr ("Lingeling", "[lingeling] ", stdout);
    fflush (stdout);
  }
  BTOR_CNEW (smgr->mm, res);
  res->lgl = lglminit (smgr->mm,
                       (lglalloc) btor_malloc,
                       (lglrealloc) btor_realloc,
                       (lgldealloc) btor_free);
  assert (res);
  if (smgr->optstr)
    btor_passdown_lingeling_options (smgr, smgr->optstr, res->lgl);
  return res;
}

static void
btor_lingeling_add (BtorSATMgr *smgr, int lit)
{
  BtorLGL *blgl = smgr->solver;
  lgladd (blgl->lgl, lit);
}

static int
btor_lingeling_sat (BtorSATMgr *smgr, int limit)
{
  BtorLGL *blgl = smgr->solver;
  LGL *lgl      = blgl->lgl, *forked, *bforked;
  int res, fres, bfres;
  const int blimit = 20000;
  char name[80];

  if (!smgr->nofork || (limit >= 0 && (limit < blimit)))
  {
    if (limit < INT_MAX) lglsetopt (lgl, "clim", limit);
    res = lglsat (lgl);
  }
  else
  {
    lglsetopt (lgl, "clim", blimit);
    if (!(res = lglsat (lgl)))
    {
      blgl->nbforked++;
      bforked = lglbrutefork (lgl, 0);
      lglsetopt (bforked, "seed", blgl->nbforked);
      sprintf (name, "[lglbrutefork%d] ", blgl->nbforked);
      lglsetprefix (bforked, name);
      lglsetout (bforked, smgr->output);
      if (lglgetopt (lgl, "verbose")) lglsetopt (bforked, "verbose", 1);
      if (limit >= 0 && limit < INT_MAX) lglsetopt (bforked, "clim", limit);
      res = lglsat (bforked);
      if (smgr->verbosity > 0) lglstats (bforked);
      bfres = lgljoin (lgl, bforked);
      assert (!res || bfres == res);
      res = bfres;
    }
  }
  return res;
}

static int
btor_lingeling_changed (BtorSATMgr *smgr)
{
  BtorLGL *blgl = smgr->solver;
  return lglchanged (blgl->lgl);
}

static int
btor_lingeling_deref (BtorSATMgr *smgr, int lit)
{
  BtorLGL *blgl = smgr->solver;
  return lglderef (blgl->lgl, lit);
}

static int
btor_lingeling_repr (BtorSATMgr *smgr, int lit)
{
  BtorLGL *blgl = smgr->solver;
  return lglrepr (blgl->lgl, lit);
}

static void
btor_lingeling_reset (BtorSATMgr *smgr)
{
  BtorLGL *blgl = smgr->solver;
  lglrelease (blgl->lgl);
  BTOR_DELETE (smgr->mm, blgl);
}

static void
btor_lingeling_set_output (BtorSATMgr *smgr, FILE *output)
{
  BtorLGL *blgl = smgr->solver;
  lglsetout (blgl->lgl, output);
}

static void
btor_lingeling_set_prefix (BtorSATMgr *smgr, const char *prefix)
{
  BtorLGL *blgl = smgr->solver;
  lglsetprefix (blgl->lgl, prefix);
}

static void
btor_lingeling_enable_verbosity (BtorSATMgr *smgr)
{
  BtorLGL *blgl = smgr->solver;
  lglsetopt (blgl->lgl, "verbose", 1);
}

static int
btor_lingeling_inc_max_var (BtorSATMgr *smgr)
{
  BtorLGL *blgl = smgr->solver;
  int res       = lglincvar (blgl->lgl);
  // TODO what about this?
  // if (smgr->need)
  lglfreeze (blgl->lgl, res);
  return res;
}

static int
btor_lingeling_variables (BtorSATMgr *smgr)
{
  BtorLGL *blgl = smgr->solver;
  return lglmaxvar (blgl->lgl);
}

static void
btor_lingeling_stats (BtorSATMgr *smgr)
{
  BtorLGL *blgl = smgr->solver;
  lglstats (blgl->lgl);
  btor_msg_sat (
      smgr, 1, "%d forked, %d brute forked", blgl->nforked, blgl->nbforked);
}

/*------------------------------------------------------------------------*/

static void
btor_lingeling_assume (BtorSATMgr *smgr, int lit)
{
  BtorLGL *blgl = smgr->solver;
  lglassume (blgl->lgl, lit);
}

static void
btor_lingeling_melt (BtorSATMgr *smgr, int lit)
{
  BtorLGL *blgl = smgr->solver;
  lglmelt (blgl->lgl, lit);
}

static int
btor_lingeling_failed (BtorSATMgr *smgr, int lit)
{
  BtorLGL *blgl = smgr->solver;
  return lglfailed (blgl->lgl, lit);
}

static int
btor_lingeling_fixed (BtorSATMgr *smgr, int lit)
{
  BtorLGL *blgl = smgr->solver;
  return lglfixed (blgl->lgl, lit);
}

static int
btor_lingeling_inconsistent (BtorSATMgr *smgr)
{
  BtorLGL *blgl = smgr->solver;
  return lglinconsistent (blgl->lgl);
}

/*------------------------------------------------------------------------*/

int
btor_enable_lingeling_sat (BtorSATMgr *smgr,
                           const char *optstr,
                           int nofork,
                           int nobrutefork)
{
  assert (smgr != NULL);

  BTOR_ABORT_SAT (smgr->initialized,
                  "'btor_init_sat' called before 'btor_enable_lingeling_sat'");

  if ((smgr->optstr = optstr)
      && !btor_passdown_lingeling_options (smgr, optstr, 0))
    return 0;

  smgr->name        = "Lingeling";
  smgr->nofork      = nofork;
  smgr->nobrutefork = nobrutefork;

  smgr->api.add              = btor_lingeling_add;
  smgr->api.assume           = btor_lingeling_assume;
  smgr->api.changed          = btor_lingeling_changed;
  smgr->api.deref            = btor_lingeling_deref;
  smgr->api.enable_verbosity = btor_lingeling_enable_verbosity;
  smgr->api.failed           = btor_lingeling_failed;
  smgr->api.fixed            = btor_lingeling_fixed;
  smgr->api.inc_max_var      = btor_lingeling_inc_max_var;
  smgr->api.inconsistent     = btor_lingeling_inconsistent;
  smgr->api.init             = btor_lingeling_init;
  smgr->api.melt             = btor_lingeling_melt;
  smgr->api.repr             = btor_lingeling_repr;
  smgr->api.reset            = btor_lingeling_reset;
  smgr->api.sat              = btor_lingeling_sat;
  smgr->api.set_output       = btor_lingeling_set_output;
  smgr->api.set_prefix       = btor_lingeling_set_prefix;
  smgr->api.stats            = btor_lingeling_stats;
  smgr->api.variables        = btor_lingeling_variables;

  btor_msg_sat (
      smgr, 1, "Lingeling allows both incremental and non-incremental mode");

  return 1;
}
#endif

/*------------------------------------------------------------------------*/

#ifdef BTOR_USE_MINISAT

/*------------------------------------------------------------------------*/

void
btor_enable_minisat_sat (BtorSATMgr *smgr)
{
  assert (smgr != NULL);

  BTOR_ABORT_SAT (smgr->initialized,
                  "'btor_init_sat' called before 'btor_enable_minisat_sat'");

  smgr->name   = "MiniSAT";
  smgr->optstr = 0;

  smgr->api.add              = btor_minisat_add;
  smgr->api.assume           = btor_minisat_assume;
  smgr->api.changed          = btor_minisat_changed;
  smgr->api.deref            = btor_minisat_deref;
  smgr->api.enable_verbosity = btor_minisat_enable_verbosity;
  smgr->api.failed           = btor_minisat_failed;
  smgr->api.fixed            = btor_minisat_fixed;
  smgr->api.inc_max_var      = btor_minisat_inc_max_var;
  smgr->api.inconsistent     = btor_minisat_inconsistent;
  smgr->api.init             = btor_minisat_init;
  smgr->api.melt             = 0;
  smgr->api.repr             = btor_minisat_repr;
  smgr->api.reset            = btor_minisat_reset;
  smgr->api.sat              = btor_minisat_sat;
  smgr->api.set_output       = btor_minisat_set_output;
  smgr->api.set_prefix       = btor_minisat_set_prefix;
  smgr->api.stats            = btor_minisat_stats;
  smgr->api.variables        = btor_minisat_variables;

  btor_msg_sat (
      smgr, 1, "MiniSAT allows both incremental and non-incremental mode");
}

#endif

/*------------------------------------------------------------------------*/
