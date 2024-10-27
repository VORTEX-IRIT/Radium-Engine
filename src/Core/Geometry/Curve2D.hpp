#pragma once

#include <Core/Containers/VectorArray.hpp>
#include <Core/Geometry/Spline.hpp>
#include <Core/RaCore.hpp>
#include <vector>

namespace Ra {
namespace Core {
namespace Geometry {
class RA_CORE_API Curve2D
{
  public:
    enum CurveType { LINE, CUBICBEZIER, SPLINE, SIZE };

    using Vector = Eigen::Matrix<Scalar, 2, 1>;

    virtual void addPoint( const Vector p ) = 0;
    virtual ~Curve2D()                      = default;

    virtual Vector f( Scalar u ) const                 = 0;
    virtual Vector df( Scalar u ) const                = 0;
    virtual Vector fdf( Scalar t, Vector& grad ) const = 0;

  protected:
    int size;
};

class RA_CORE_API QuadraSpline : public Curve2D
{
  public:
    QuadraSpline() { this->size = 0; }
    QuadraSpline( const Curve2D::Vector& p0,
                  const Curve2D::Vector& p1,
                  const Curve2D::Vector& p2 ) :
        m_points { p0, p1, p2 } {
        this->size = 3;
    }
    ~QuadraSpline() override = default;

    inline void addPoint( const Vector p ) override;

    inline Vector f( Scalar u ) const override;
    inline Vector df( Scalar u ) const override;
    inline Vector fdf( Scalar u, Vector& grad ) const override;

  private:
    Core::VectorArray<Vector> m_points;
};

class RA_CORE_API CubicBezier : public Curve2D
{
  public:
    CubicBezier() { this->size = 0; }
    CubicBezier( const Curve2D::Vector& p0,
                 const Curve2D::Vector& p1,
                 const Curve2D::Vector& p2,
                 const Curve2D::Vector& p3 ) :
        m_points { p0, p1, p2, p3 } {
        this->size = 4;
    }
    ~CubicBezier() override = default;

    inline void addPoint( const Vector p ) override;

    inline Vector f( Scalar u ) const override;
    inline Vector df( Scalar u ) const override;
    inline Vector fdf( Scalar t, Vector& grad ) const override;

    /**
     * @brief Computes the cubic Bernstein coefficients for parameter t
     * @param t parameter of the coefficients
     * @param deriv derivative order
     * @return a vector of 4 scalar coefficients
     */
    static std::vector<float> bernsteinCoefsAt( float u, int deriv = 0 );

    /**
     * @brief get a list of curviline abscisses
     * @param distance in cm accross the curve that separate two params value
     * @param step sampling [0, 1]
     * @return list of params [0, 1]
     */
    std::vector<float> getArcLengthParameterization( float resolution, float epsilon ) const;

    const VectorArray<Vector> getCtrlPoints() const;

  private:
    Vector m_points[4];
};

class RA_CORE_API Line : public Curve2D
{
  public:
    Line() { this->size = 0; }
    Line( const Vector& p0, const Vector& p1 ) : m_points { p0, p1 } { this->size = 2; }
    ~Line() override = default;

    inline void addPoint( const Vector p ) override;

    inline Vector f( Scalar u ) const override;
    inline Vector df( Scalar u ) const override;
    inline Vector fdf( Scalar t, Vector& grad ) const override;

  private:
    Vector m_points[2];
};

class RA_CORE_API SplineCurve : public Curve2D
{
  public:
    SplineCurve() { this->size = 0; }
    explicit SplineCurve( Core::VectorArray<Vector> points ) : m_points( points ) {
        this->size = points.size();
    }
    ~SplineCurve() override = default;

    inline void addPoint( const Vector p ) override;

    inline Vector f( Scalar u ) const override;
    inline Vector df( Scalar u ) const override;
    inline Vector fdf( Scalar t, Vector& grad ) const override;

  private:
    Core::VectorArray<Vector> m_points;
};

class RA_CORE_API PiecewiseCubicBezier : public Curve2D
{
  public:
    PiecewiseCubicBezier() {}

    /**
     * @brief Spline of cubic Bézier segments. Construction guarantees C0 continuity.
     *        ie extremities of successive segments share the same coordinates
     * @param vector of control points, should be 3*n+1 points where n is the number of segments
     */
    PiecewiseCubicBezier( const Core::VectorArray<Vector>& cpoints ) { setCtrlPoints( cpoints ); }

    PiecewiseCubicBezier( const PiecewiseCubicBezier& other ) {
        setCtrlPoints( other.getCtrlPoints() );
    }

    int getNbBezier() const { return m_spline.size(); }

    const std::vector<CubicBezier> getSplines() const { return m_spline; }

    /**
     * @brief Computes a sample point in the bezier spline
     * @param u global parameter of the sample, should be in [0,nbz]
     *        integer part of u represents the id of the Bézier segment
     *        while decimal part of u represents the local Bézier parameter
     * @param deriv derivative order of the sampling
     * @return coordinates of the sample point
     */
    inline Vector f( float u ) const override;

    /**
     * @brief Computes a list of samples points in the bezier spline
     * @param list of u global parameter of the sample, should be in [0,nbz]
     *        integer part of u represents the id of the Bézier segment
     *        while decimal part of u represents the local Bézier parameter
     * @param deriv derivative order of the sampling
     * @return coordinates of the sample point
     */
    inline VectorArray<Vector> f( std::vector<float> params ) const;

    inline Vector df( float u ) const override;

    inline Vector fdf( Scalar t, Vector& grad ) const override;

    inline void addPoint( const Vector p ) override;

    VectorArray<Vector> getCtrlPoints() const;

    void setCtrlPoints( const VectorArray<Vector>& cpoints );

    /**
     * @brief Decomposes a spline global parameter into the local Bézier parameters (static)
     * @param global parameter
     * @param number of segments in the spline
     * @return a pair (b,t) where b is the index of the bezier segment, and t the local parameter in
     * the segment
     */
    static std::pair<int, float> getLocalParameter( float u, int nbz );

    /**
     * @brief Map a normalized parameter for the spline to a global parameter
     * @param normalized parameter [0, 1]
     * @param number of segments in the spline
     * @return a global parameter t [0, nbz]
     */
    static float getGlobalParameter( float u, int nbz );

    /**
     * @brief equivalent to linspace function
     * @param number of param
     * @return a list of parameters t [0, nbz]
     */
    std::vector<float> getUniformParameterization( int nbSamples ) const;

    /**
     * @brief get a list of curviline abscisses
     * @param distance in cm accross the curve that separate two params value
     * @param step sampling [0, 1]
     * @return list of params [0, nbz]
     */
    std::vector<float> getArcLengthParameterization( float resolution, float epsilon ) const;

  private:
    std::vector<CubicBezier> m_spline; // Vector of Bézier segments in the spline

    std::pair<int, float> getLocalParameter( float u ) const {
        return getLocalParameter( u, getNbBezier() );
    }
};

/*--------------------------------------------------*/

void CubicBezier::addPoint( const Curve2D::Vector p ) {
    if ( size < 4 ) { m_points[size++] = p; }
}

Curve2D::Vector CubicBezier::f( Scalar u ) const {
    Vector grad;
    return fdf( u, grad );
}

Curve2D::Vector CubicBezier::df( Scalar u ) const {
    Vector grad;
    fdf( u, grad );
    return grad;
}

Curve2D::Vector CubicBezier::fdf( Scalar t, Vector& grad ) const {
    float t2         = t * t;
    float t3         = t2 * t;
    float oneMinusT  = 1.0 - t;
    float oneMinusT2 = oneMinusT * oneMinusT;
    float oneMinusT3 = oneMinusT2 * oneMinusT;

    grad = 3.0 * oneMinusT2 * ( m_points[1] - m_points[0] ) +
           6.0 * oneMinusT * t * ( m_points[2] - m_points[1] ) +
           3.0 * t2 * ( m_points[3] - m_points[2] );
    return oneMinusT3 * m_points[0] + 3.0 * oneMinusT2 * t * m_points[1] +
           3.0 * oneMinusT * t2 * m_points[2] + t3 * m_points[3];
}

/*--------------------------------------------------*/

void Line::addPoint( const Curve2D::Vector p ) {
    if ( size < 2 ) { m_points[size++] = p; }
}

Curve2D::Vector Line::f( Scalar u ) const {
    return ( 1.0 - u ) * m_points[0] + u * m_points[1];
}

Curve2D::Vector Line::df( Scalar /*u*/ ) const {
    return m_points[1] - m_points[0];
}

Curve2D::Vector Line::fdf( Scalar t, Vector& grad ) const {
    grad = m_points[1] - m_points[0];
    return ( 1.0 - t ) * m_points[0] + t * m_points[1];
}

/*--------------------------------------------------*/

void SplineCurve::addPoint( const Curve2D::Vector p ) {
    m_points.push_back( p );
    ++size;
}

Curve2D::Vector SplineCurve::f( Scalar u ) const {
    Spline<2, 3> spline;
    spline.setCtrlPoints( m_points );

    return spline.f( u );
}

Curve2D::Vector SplineCurve::df( Scalar u ) const {
    Spline<2, 3> spline;
    spline.setCtrlPoints( m_points );

    return spline.df( u );
}

Curve2D::Vector SplineCurve::fdf( Scalar u, Curve2D::Vector& grad ) const {
    Spline<2, 3> spline;
    spline.setCtrlPoints( m_points );

    grad = spline.df( u );
    return spline.f( u );
}

/*--------------------------------------------------*/

void QuadraSpline::addPoint( const Curve2D::Vector p ) {
    m_points.push_back( p );
    ++size;
}

Curve2D::Vector QuadraSpline::f( Scalar u ) const {
    Spline<2, 2> spline;
    spline.setCtrlPoints( m_points );

    return spline.f( u );
}

Curve2D::Vector QuadraSpline::df( Scalar u ) const {
    Spline<2, 2> spline;
    spline.setCtrlPoints( m_points );

    return spline.df( u );
}

Curve2D::Vector QuadraSpline::fdf( Scalar u, Vector& grad ) const {
    Spline<2, 2> spline;
    spline.setCtrlPoints( m_points );

    grad = spline.df( u );
    return spline.f( u );
}

/*--------------------------------------------------*/

Curve2D::Vector PiecewiseCubicBezier::f( float u ) const {
    std::pair<int, float> locpar { getLocalParameter( u ) };

    if ( locpar.first < 0 || locpar.first > getNbBezier() - 1 ) {
        Vector p;
        p.fill( 0 );
        return p;
    }

    return m_spline[locpar.first].f( locpar.second );
}

VectorArray<Curve2D::Vector> PiecewiseCubicBezier::f( std::vector<float> params ) const {
    VectorArray<Curve2D::Vector> controlPoints;

    for ( int i = 0; i < (int)params.size(); ++i ) {
        controlPoints.push_back( f( params[i] ) );
    }

    return controlPoints;
}

Curve2D::Vector PiecewiseCubicBezier::df( float u ) const {
    std::pair<int, float> locpar { getLocalParameter( u ) };

    if ( locpar.first < 0 || locpar.first > getNbBezier() - 1 ) {
        Vector p;
        p.fill( 0 );
        return p;
    }

    return m_spline[locpar.first].df( locpar.second );
}

Curve2D::Vector PiecewiseCubicBezier::fdf( Scalar t, Vector& grad ) const {
    std::pair<int, float> locpar { getLocalParameter( t ) };

    if ( locpar.first < 0 || locpar.first > getNbBezier() - 1 ) {
        Vector p;
        p.fill( 0 );
        return p;
    }

    return m_spline[locpar.first].fdf( locpar.second, grad );
}

void PiecewiseCubicBezier::addPoint( const Curve2D::Vector p ) {
    if ( m_spline[m_spline.size() - 1].getCtrlPoints().size() < 4 )
        m_spline[m_spline.size() - 1].addPoint( p );
}

} // namespace Geometry
} // namespace Core
} // namespace Ra
