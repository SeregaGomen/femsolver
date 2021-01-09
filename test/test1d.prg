#mesh body1d.trpa
argument x
result u
constant E = 203200
function Exx, Sxx
load X = 0
functional W

Exx = diff(u, x)
Sxx = E * Exx

W = 0.5 * (integral(Sxx var Exx) - integral(X var u))

u(x == 0) = 0

X(x == 10) = 1
