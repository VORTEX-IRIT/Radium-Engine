#include <Core/Containers/VectorArray.hpp>
#include <Core/Geometry/Curve2D.hpp>

namespace Ra {
namespace Core {
namespace Geometry {

/*--------------------------------------------------*/

std::vector<float> CubicBezier::bernsteinCoefsAt( float u, int deriv ) {
    if ( deriv == 2 ) { return { 6 * ( 1 - u ), 6 * ( -2 + 3 * u ), 6 * ( 1 - 3 * u ), 6 * u }; }
    else if ( deriv == 1 )
        return { -3 * powf( 1 - u, 2 ),
                 3 * ( 1 - u ) * ( 1 - 3 * u ),
                 3 * u * ( 2 - 3 * u ),
                 3 * powf( u, 2 ) };
    else
        return { powf( 1 - u, 3 ),
                 3 * u * powf( 1 - u, 2 ),
                 3 * powf( u, 2 ) * ( 1 - u ),
                 powf( u, 3 ) };
}

std::vector<float> CubicBezier::getArcLengthParameterization( float resolution,
                                                              float epsilon ) const {
    std::vector<float> params;
    float start    = 0.0f;
    float end      = 1.0f;
    float curParam = start;
    float curDist  = 0.0f;

    params.push_back( curParam );

    Vector p0 = f( curParam );
    curParam += epsilon;

    while ( curParam <= end ) {
        Vector p1 = f( curParam );
        curDist += sqrt( pow( p0.x() - p1.x(), 2 ) + pow( p0.y() - p1.y(), 2 ) );
        if ( curDist >= resolution ) {
            params.push_back( curParam );
            curDist = 0.0f;
        }
        p0 = p1;
        curParam += epsilon;
    }

    // push last sample point to the end to ensure bounds [0, 1]
    params[params.size() - 1] = end;

    return params;
}

const VectorArray<Curve2D::Vector> CubicBezier::getCtrlPoints() const {
    VectorArray<Vector> ctrlPts;
    ctrlPts.reserve( 4 );
    ctrlPts.push_back( m_points[0] );
    ctrlPts.push_back( m_points[1] );
    ctrlPts.push_back( m_points[2] );
    ctrlPts.push_back( m_points[3] );
    return ctrlPts;
}

/*--------------------------------------------------*/

std::pair<int, float> PiecewiseCubicBezier::getLocalParameter( float u, int nbz ) {
    int b { (int)( std::floor( u ) ) };
    float t { u - b };

    if ( ( b == nbz ) && ( t == 0 ) ) {
        b = nbz - 1;
        t = 1;
    }
    return { b, t };
}

float PiecewiseCubicBezier::getGlobalParameter( float u, int nbz ) {
    return u * nbz;
}

VectorArray<Curve2D::Vector> PiecewiseCubicBezier::getCtrlPoints() const {
    VectorArray<Curve2D::Vector> cp;
    cp.reserve( 3 * getNbBezier() + 1 );
    for ( unsigned int i = 0; i < m_spline.size(); i++ ) {
        cp.push_back( m_spline[i].getCtrlPoints()[0] );
        cp.push_back( m_spline[i].getCtrlPoints()[1] );
        cp.push_back( m_spline[i].getCtrlPoints()[2] );
    }
    if ( !m_spline.empty() ) { cp.push_back( m_spline[m_spline.size() - 1].getCtrlPoints()[3] ); }

    return cp;
}

void PiecewiseCubicBezier::setCtrlPoints( const VectorArray<Curve2D::Vector>& cpoints ) {
    int nbz { (int)( ( cpoints.size() - 1 ) / 3 ) };
    m_spline.clear();
    m_spline.reserve( nbz );
    for ( int b = 0; b < nbz; ++b ) {
        m_spline.emplace_back( CubicBezier(
            cpoints[3 * b], cpoints[3 * b + 1], cpoints[3 * b + 2], cpoints[3 * b + 3] ) );
    }
}

std::vector<float> PiecewiseCubicBezier::getUniformParameterization( int nbSamples ) const {
    std::vector<float> params;
    params.resize( nbSamples );

    float delta = { 1.0f / (float)( nbSamples - 1 ) };
    float acc   = 0.0f;
    int nbz     = getNbBezier();

    params[0] = getGlobalParameter( 0.0f, nbz );
    for ( int i = 1; i < nbSamples; ++i ) {
        acc += delta;
        params[i] = getGlobalParameter( acc, nbz );
    }

    params[params.size() - 1] = getGlobalParameter( 1.0f, nbz );

    return params;
}

std::vector<float> PiecewiseCubicBezier::getArcLengthParameterization( float resolution,
                                                                       float epsilon ) const {

    std::vector<float> params;
    int nbz = getNbBezier();

    if ( nbz <= 0 ) return params;

    params = m_spline[0].getArcLengthParameterization( resolution, epsilon );

    for ( int i = 1; i < nbz; ++i ) {
        std::vector<float> tmpParams =
            m_spline[i].getArcLengthParameterization( resolution, epsilon );
        std::transform( tmpParams.begin(),
                        tmpParams.end(),
                        tmpParams.begin(),
                        [&]( auto const& elem ) { return elem + i; } );
        params.insert( params.end(), tmpParams.begin() + 1, tmpParams.end() );
    }
    return params;
}

} // namespace Geometry
} // namespace Core
} // namespace Ra
