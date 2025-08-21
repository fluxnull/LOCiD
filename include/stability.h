#ifndef STABILITY_H
#define STABILITY_H

#include "loci.h"

// Computes the stability rank (High, Medium, Low) for a given set of loci.
// Returns a string that needs to be freed by the caller.
char* compute_stability_rank(const Loci* loci);

#endif // STABILITY_H
