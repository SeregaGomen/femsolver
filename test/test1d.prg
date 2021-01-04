#mesh body1d.trpa
argument x
result U
constant E = 203200
function Exx, Sxx
load X = 0
functional W

Exx = diff(U, x)
Sxx = E * Exx
W = 0.5 * (integral(Sxx var Exx) - integral(X var U))
U(x == 0) = 0
X(x == 10) = 1
