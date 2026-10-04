#pragma once

#include "VisConstants.h"

#include <array>

namespace wcv {

// Unnormalised in-place real FFT of 512 samples, the role of fft_9() in
// Src/Winamp/FFT.cpp. The repo has only the nsutil header, not its body.
// Output uses the packed "Perm" layout that makeSpecData reads in pairs:
// [Re0, Re256, Re1, Im1, Re2, Im2, ..., Re255, Im255].
void forwardFft512(std::array<float, kFftSize>& signal);

} // namespace wcv
