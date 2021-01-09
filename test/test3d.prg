#mesh cube4.trpa
argument x, y, z
result u, v, w
constant E = 203200, m = 0.27, G = E / (2 + 2 * m), L = 2 * m * G / (1 - 2 * m)
function Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz
load X = 0, Y = 0, Z = 0.5
functional W

Exx = diff(u, x)
Eyy = diff(v, y)
Ezz = diff(w, z)
Exy = diff(u, y) + diff(v, x)
Exz = diff(u, z) + diff(w, x)
Eyz = diff(v, z) + diff(w, y)

Sxx = 2 * G * Exx + L * (Exx + Eyy + Ezz)
Syy = 2 * G * Eyy + L * (Exx + Eyy + Ezz)
Szz = 2 * G * Ezz + L * (Exx + Eyy + Ezz)
Sxy = G * Exy
Sxz = G * Exz
Syz = G * Eyz

W = 0.5 * integral(Sxx var Exx + Syy var Eyy + Szz var Ezz + Sxy var Exy + Sxz var Exz + Syz var Eyz) - integral(X var u + Y var w + Z var w)

u(z == 0) = 0
v(z == 0) = 0
w(z == 0) = 0

//Z(z == 1) = -1
