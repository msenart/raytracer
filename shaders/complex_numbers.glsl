#ifndef COMPLEX_NUMBERS_GLSL
#define COMPLEX_NUMBERS_GLSL

struct cx {
    float a;
    float b;
};

cx csum(cx x, cx y){
    return cx(x.a + y.a, x.b + y.b);
}

cx cdiff(cx x, cx y){
    return cx(x.a - y.a, x.b - y.b);
}

cx conj(cx x){
    return cx(x.a, -x.b);
}

float norm2(cx x){
    return x.a*x.a + x.b*x.b;
}

cx csqrt(cx z) {
    float r = sqrt(norm2(z));
    return cx(sqrt(max(0.5*(r + z.a), 0.0)),
            (z.b >= 0.0 ? 1.0 : -1.0) * sqrt(max(0.5*(r - z.a), 0.0)));
}

cx cmul(cx x, cx y){
    return cx(x.a*y.a - x.b*y.b, x.a*y.b + x.b * y.a);
}

cx cmul(float lam, cx x){
    return cx(x.a*lam, x.b*lam);
}

cx cmul(cx x, float lam){
    return cx(x.a*lam, x.b*lam);
}

cx cdiv(cx x, cx y){
    return cmul(1.f/norm2(y), cx(x.a*y.a + x.b*y.b, x.b * y.a - x.a*y.b));
}

#endif
