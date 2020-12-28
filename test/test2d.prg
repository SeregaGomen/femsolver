#mesh body2d.trpa
argument x, y
result u, v
constant E = 203200, m = 0.3, K = E / (1 - 2 * m), G = E / (2 + 2 * m)
function Exx, Eyy, Exy, Sxx, Syy, Sxy
load X, Y
functional W

Exx = diff(u, x)
Eyy = diff(v, y)
Exy = diff(u, y) + diff(v, x)

Sxx = K * (Exx + m * Eyy)
Syy = K * (m * Exx + Eyy)
Sxy = G * Exy

W = 0.5 * integral(Sxx var Exx + Syy var Eyy + Sxy var Exy)

u(x == 0) = 0
v(x == 0) = 0

X(x == 1) = 1
