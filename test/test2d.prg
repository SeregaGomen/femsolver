#mesh console4.trpa
//#mesh console.trpa
//#mesh body2d.trpa
argument x, y
result u, v
constant E = 203200, m = 0.27, K = E / (1 - m * m), G = E / (2 + 2 * m)
function Exx, Eyy, Exy, Sxx, Syy, Sxy
load X = 0, Y = -1
functional W

Exx = diff(u, x)
Eyy = diff(v, y)
Exy = diff(u, y) + diff(v, x)

Sxx = K * (Exx + m * Eyy)
Syy = K * (m * Exx + Eyy)
Sxy = G * Exy


W = 0.5 * integral(Sxx var Exx + Syy var Eyy + Sxy var Exy) - integral(X var u + Y var v)
//W = integral(Y var v)


u(x == 0) = 0
v(x == 0) = 0

//Y(x == 10 and y == -0.25) = 1
