// Copyright © 2021 Audulus LLC. All rights reserved.

#ifndef sdf_h
#define sdf_h

#ifdef __METAL_VERSION__
#define DEVICE device
#else
#define DEVICE

#include <simd/simd.h>
using namespace simd;

inline float min(float a, float b) {
    return a > b ? b : a;
}

inline float max(float a, float b) {
    return a > b ? a : b;
}

inline float2 max(float2 a, float b) {
    return simd_max(a, float2{b,b});
}

inline float clamp(float x, float a, float b) {
    if(x > b) x = b;
    if(x < a) x = a;
    return x;
}

inline float3 clamp(float3 x, float a, float b) {
    return simd_clamp(x, a, b);
}

inline float mix(float a, float b, float t) {
    return (1-t)*a + t*b;
}

inline float2 mix(float2 a, float2 b, float t) {
    return (1-t)*a + t*b;
}

#endif

// From https://www.iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm

inline float sdCircle( float2 p, float r )
{
    return length(p) - r;
}

inline float sdBox( float2 p, float2 b, float r )
{
    float2 d = abs(p)-b+r;
    return length(max(d,float2(0.0))) + min(max(d.x,d.y),0.0)-r;
}

inline float sdSegment(float2 p, float2 a, float2 b )
{
    float2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

inline float sdArc(float2 p, float2 sca, float2 scb, float ra, float rb )
{
    p = p * float2x2{float2{sca.x,sca.y},float2{-sca.y,sca.x}};
    p.x = abs(p.x);
    float k = (scb.y*p.x>scb.x*p.y) ? dot(p,scb) : length(p);
    return sqrt( dot(p,p) + ra*ra - 2.0*ra*k ) - rb;
}

inline float sdHorseshoe(float2 p, float2 c, float r, float2 w )
{
    p.x = abs(p.x);
    float l = length(p);
    p = float2x2{float2{-c.x, c.y},float2{c.y, c.x}}*p;
    p = float2{(p.y>0.0)?p.x:l*sign(-c.x), (p.x>0.0)?p.y:l };
    p = float2{p.x,abs(p.y-r)}-w;
    return length(max(p,0.0f)) + min(0.0f,max(p.x,p.y));
}

inline float dot2(float2 v) {
    return dot(v,v);
}

inline float sdBezier(float2 pos, float2 A, float2 B, float2 C )
{
    float2 a = B - A;
    float2 b = A - 2.0*B + C;
    float2 c = a * 2.0;
    float2 d = A - pos;
    float kk = 1.0/dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b)) / 3.0;
    float kz = kk * dot(d,a);
    float res = 0.0;
    float p = ky - kx*kx;
    float p3 = p*p*p;
    float q = kx*(2.0*kx*kx-3.0*ky) + kz;
    float h = q*q + 4.0*p3;
    if( h >= 0.0)
    {
        h = sqrt(h);
        float2 x = (float2{h,-h}-q)/2.0;
        float2 uv = sign(x)*pow(abs(x), float2(1.0/3.0));
        float t = clamp( uv.x+uv.y-kx, 0.0, 1.0 );
        res = dot2(d + (c + b*t)*t);
    }
    else
    {
        float z = sqrt(-p);
        float v = acos( q/(p*z*2.0) ) / 3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        float3  t = clamp(float3{m+m,-n-m,n-m}*z-kx,0.0,1.0);
        res = min( dot2(d+(c+b*t.x)*t.x),
                   dot2(d+(c+b*t.y)*t.y) );
        // the third root cannot be the closest
        // res = min(res,dot2(d+(c+b*t.z)*t.z));
    }
    return sqrt( res );
}

// From https://www.shadertoy.com/view/4sySDK

inline float2x2 inv(float2x2 M) {
    return (1.0f / determinant(M)) * float2x2{
        float2{M.columns[1][1], -M.columns[0][1]},
        float2{-M.columns[1][0], M.columns[0][0]}};
}

inline float sdBezier2(float2 uv, float2 p0, float2 p1, float2 p2){

    const float2x2 trf1 = float2x2{ float2{-1, 2}, float2{1, 2} };
    float2x2 trf2 = inv(float2x2{p0-p1, p2-p1});
    float2x2 trf=trf1*trf2;

    uv-=p1;
    float2 xy=trf*uv;
    xy.y-=1.;

    float2 gradient;
    gradient.x=2.*trf.columns[0][0]*(trf.columns[0][0]*uv.x+trf.columns[1][0]*uv.y)-trf.columns[0][1];
    gradient.y=2.*trf.columns[1][0]*(trf.columns[0][0]*uv.x+trf.columns[1][0]*uv.y)-trf.columns[1][1];

    return (xy.x*xy.x-xy.y)/length(gradient);
}

inline float det(float2 a, float2 b) { return a.x*b.y-b.x*a.y; }

inline float2 closestPointInSegment( float2 a, float2 b )
{
    float2 ba = b - a;
    return a + ba*clamp( -dot(a,ba)/dot(ba,ba), 0.0, 1.0 );
}

// From: http://research.microsoft.com/en-us/um/people/hoppe/ravg.pdf
inline float2 get_distance_vector(float2 b0, float2 b1, float2 b2) {
    
    float a=det(b0,b2), b=2.0*det(b1,b0), d=2.0*det(b2,b1);
    
    if( abs(2.0*a+b+d) < 0.001 ) return closestPointInSegment(b0,b2);
    
    float f=b*d-a*a; // ð‘“(ð‘)
    float2 d21=b2-b1, d10=b1-b0, d20=b2-b0;
    float2 gf=2.0*(b*d21+d*d10+a*d20);
    gf=float2{gf.y,-gf.x};
    float2 pp=-f*gf/dot(gf,gf);
    float2 d0p=b0-pp;
    float ap=det(d0p,d20), bp=2.0*det(d10,d0p);
    // (note that 2*ap+bp+dp=2*a+b+d=4*area(b0,b1,b2))
    float t=clamp((ap+bp)/(2.0*a+b+d), 0.0 ,1.0);
    return mix(mix(b0,b1,t),mix(b1,b2,t),t);
    
}

inline float sdBezierApprox(float2 p, float2 b0, float2 b1, float2 b2) {
    return length(get_distance_vector(b0-p, b1-p, b2-p));
}

inline float sdWire(float2 p, float2 a, float2 b) {

    float2 sz = b-a;
    float2 uv = (p-a)/sz;
    float xscale = 5.0f;
    float x = uv.x - 0.5;
    float y = 0.5*(tanh(xscale * x) + 1);
    float c = cosh(xscale * x);
    float dydx = xscale / (c * c);
    float dy = abs(uv.y - y);

    return sz.y * dy / sqrt(1.0 + dydx * dydx);
}

#if 0
template<class T>
float sdPolygon(float2 p, T v, int num)
{
    float d = dot(p-v[0],p-v[0]);
    float s = 1.0;
    for( int i=0, j=num-1; i<num; j=i, i++ )
    {
        // distance
        float2 e = v[j] - v[i];
        float2 w =    p - v[i];
        float2 b = w - e*clamp( dot(w,e)/dot(e,e), 0.0, 1.0 );
        d = min( d, dot(b,b) );

        // winding number from http://geomalgorithms.com/a03-_inclusion.html
        bool3 cond = bool3( p.y>=v[i].y,
                            p.y <v[j].y,
                            e.x*w.y>e.y*w.x );
        if( all(cond) || all(not(cond)) ) s=-s;
    }

    return s*sqrt(d);
}
#endif

/// Axis-aligned bounding box.
struct BBox {
    float2 min;
    float2 max;

    BBox inset(float d) const {
        return {min+d, max-d};
    }

    float2 size() const { return max - min; }
};

inline BBox sdPrimBounds(const DEVICE vgerPrim& prim, const DEVICE float2* cvs) {
    BBox b;
    switch(prim.type) {
        case vgerBezier:
            b = BBox{
                min(min(prim.cvs[0], prim.cvs[1]), prim.cvs[2]),
                max(max(prim.cvs[0], prim.cvs[1]), prim.cvs[2])
            };
            break;
        case vgerCircle:
        case vgerArc:
            b = {prim.cvs[0] - prim.radius, prim.cvs[0] + prim.radius};
            break;
        case vgerSegment:
        case vgerWire:
            b = BBox{
                min(prim.cvs[0], prim.cvs[1]),
                max(prim.cvs[0], prim.cvs[1])
            };
            break;
        case vgerRect:
        case vgerRectStroke:
        case vgerGlyph:
            b = BBox{prim.cvs[0], prim.cvs[1]};
            break;
        case vgerCurve: {
            b = {FLT_MAX, -FLT_MAX};
            for(int i=0;i<prim.count;++i) {
                b.min = min(b.min, prim.cvs[i]);
                b.max = max(b.max, prim.cvs[i]);
            }
            break;
        }
        case vgerPathFill: {
            b = {FLT_MAX, -FLT_MAX};
            for(int i=0;i<prim.count*3;++i) {
                b.min = min(b.min, cvs[prim.start+i]);
                b.max = max(b.max, cvs[prim.start+i]);
            }
            break;
        }
    }
    return b.inset(-prim.width);
}

inline int bezierTest(float2 A, float2 B, float2 C, float2 p);

inline float sdPrim(const DEVICE vgerPrim& prim, const DEVICE float2* cvs, float2 p) {
    float d = FLT_MAX;
    int n = 0;
    switch(prim.type) {
        case vgerBezier:
            // d = sdBezier(p, prim.cvs[0], prim.cvs[1], prim.cvs[2]);
            d = sdBezierApprox(p, prim.cvs[0], prim.cvs[1], prim.cvs[2]) - prim.width;
            break;
        case vgerCircle:
            d = sdCircle(p - prim.cvs[0], prim.radius);
            break;
        case vgerArc:
            d = sdArc(p - prim.cvs[0], prim.cvs[1], prim.cvs[2], prim.radius, prim.width/2);
            break;
        case vgerRect:
        case vgerGlyph: {
            auto center = .5*(prim.cvs[1] + prim.cvs[0]);
            auto size = prim.cvs[1] - prim.cvs[0];
            d = sdBox(p - center, .5*size, prim.radius);
        }
            break;
        case vgerRectStroke: {
            auto center = .5*(prim.cvs[1] + prim.cvs[0]);
            auto size = prim.cvs[1] - prim.cvs[0];
            d = abs(sdBox(p - center, .5*size, prim.radius)) - prim.width/2;
        }
            break;
        case vgerSegment:
            d = sdSegment(p, prim.cvs[0], prim.cvs[1]) - prim.width;
            break;
        case vgerCurve:
            for(int i=0;i<prim.count-2;i+=2) {
                d = min(d, sdBezierApprox(p,
                                          prim.cvs[i],
                                          prim.cvs[i+1],
                                          prim.cvs[i+2]));
            }
            break;
        case vgerWire:
            d = sdWire(p, prim.cvs[0], prim.cvs[1]);
            break;
        case vgerPathFill:
            for(int i=0; i<prim.count; i++) {
                int j = prim.start + 3*i;
                n += bezierTest(cvs[j], cvs[j+1], cvs[j+2], p);
            }
            if(n % 2) {
                d = 0; // completely inside
            }
            // Outside, calculate stroke.
            for(int i=0; i<prim.count; i++) {
                int j = prim.start + 3*i;
                d = min(d, sdBezierApprox(p, cvs[j], cvs[j+1], cvs[j+2]));
            }
            break;
        default:
            break;
    }
    return d;
}

// Oriented bounding box.
struct OBB {
    float2 origin;
    float2 u;
    float2 v;

    OBB inset(float d) const {
        auto un = normalize(u);
        auto vn = normalize(v);
        return {origin+d*(un+vn), u-2*d*un, v-2*d*vn};
    }
};

// Projection of b onto a.
inline float2 proj(float2 a, float2 b) {
    return normalize(a) * dot(a,b) / length(a);
}

inline float2 orth(float2 a, float2 b) {
    return b - proj(a, b);
}

inline float2 rot90(float2 p) {
    return {-p.y, p.x};
}

inline OBB sdPrimOBB(const DEVICE vgerPrim& prim) {
    switch(prim.type) {
        case vgerBezier: {
            auto o = prim.cvs[0];
            auto u = prim.cvs[2]-o;
            auto v = orth(prim.cvs[2]-o, prim.cvs[1]-o);
            return { o, u, v };
        }
        case vgerCircle: {
            auto d = 2*prim.radius;
            return { prim.cvs[0] - prim.radius, {d,0}, {0,d} };
        }
        case vgerRect: {
            auto sz = prim.cvs[1]-prim.cvs[0];
            return { prim.cvs[0], {sz.x,0}, {0,sz.y} };
        }
        case vgerSegment: {
            auto a = prim.cvs[0];
            auto u = prim.cvs[1] - prim.cvs[0];
            auto v = rot90(u)*.001;
            return { a, u, v };
        }
        case vgerCurve: {
            // XXX: not oriented
            float2 lo = FLT_MAX;
            float2 hi = FLT_MIN;
            for(int i=0;i<prim.count;++i) {
                lo = min(lo, prim.cvs[i]);
                hi = max(hi, prim.cvs[i]);
            }
            auto sz = hi-lo;
            return {lo, {sz.x,0}, {0,sz.y}};
        }
        case vgerArc: {
            auto o = prim.cvs[0];
            auto r = prim.radius;
            return { o-r, {2*r, 0}, {0, 2*r}};
        }
        default:
            break;
    }
    return {0,0};
}

inline float4 applyPaint(const DEVICE vgerPaint& paint, float2 p) {

    float d = clamp((paint.xform * float3{p.x, p.y, 1.0}).x, 0.0, 1.0);

#ifdef __METAL_VERSION__
    return mix(paint.innerColor, paint.outerColor, d);
#else
    return simd_mix(paint.innerColor, paint.outerColor, d);
#endif

}

/// Quadratic bezier curve.
inline float2 bezier(float2 A, float2 B, float2 C, float t) {
    return (1 - t) * (1 - t) * A + 2 * (1 - t) * t * B + t * t * C;
}

// From https://github.com/linebender/kurbo/blob/25ec803ecd1bb908d2b1d8242282b76c060b26d6/src/common.rs#L105

/// Find real roots of quadratic equation.
///
/// Returns values of x for which c0 + c1 x + c2 x² = 0.
inline float2 solve_quadratic(float c0, float c1, float c2) {
    float sc0 = c0 * (1.0f/c2);
    float sc1 = c1 * (1.0f/c2);
    if(isinf(c0) || isinf(sc1)) {
        // c2 is zero or very small, treat as linear eqn
        float root = -c0 / c1;
        if(!isinf(root)) {
            return float2{root, NAN};
        } else if(c0 == 0.0f || c1 == 0.0f) {
            // Degenerate.
            return float2{0.0, NAN};
        }
    }

    float arg = sc1 * sc1 - 4.0f * sc0;
    float root1;
    if(isinf(arg)) {
        root1 = -sc1;
    } else {
        if(arg < 0.0) {
            return float2{NAN, NAN};
        } else if(arg == 0.0) {
            return float2{-0.5f * sc1, NAN};
        }
        // See https://math.stackexchange.com/questions/866331
        root1 = -0.5f * (sc1 + copysign(sqrt(arg), sc1));
    }
    float root2 = sc0 / root1;
    if(!isinf(root2)) {
        if(root2 > root1) {
            return float2{root1, root2};
        } else {
            return float2{root2, root1};
        }
    } else {
        return float2{root1, NAN};
    }
    return float2{NAN, NAN};
}

/// Intersect a bezier with a horizontal line.
inline float2 bezierIntersect(float2 A, float2 B, float2 C, float y) {

    // Quadratic bezier:
    // f(t) = (1 - t) * (1 - t) * A + 2 * (1 - t) * t * B + t * t * C

    float a = C.y - 2.0 * B.y + A.y;
    float b = 2.0 * (B.y - A.y);
    float c = A.y - y;

    return solve_quadratic(c,b,a);
}

/// Returns number of intersections between quadratic bezier and x-axis ray starting at p.
inline int bezierTest(float2 A, float2 B, float2 C, float2 p) {

    int c = 0;
    auto t = bezierIntersect(A, B, C, p.y);

    for(int i=0;i<2;++i) {
        c += !isnan(t[i]) && (t[i] >= 0.0) && (t[i] < 1.0) && (bezier(A, B, C, t[i]).x > p.x);
    }

    return c;
}

#endif /* sdf_h */
