#mesh body1d.trpa
argument x
result U
constant E = 203200
function Exx, Sxx
functional W

Exx = diff(U, x)
Sxx = E * Exx
W = -integral(Sxx & Exx)
U(x == 0) = 0
