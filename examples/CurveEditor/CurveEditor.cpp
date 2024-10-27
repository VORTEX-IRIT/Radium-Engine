#include <Core/Geometry/Curve2D.hpp>
#include <Core/Math/Interpolation.hpp>
#include <Engine/Data/Material.hpp>
#include <Engine/Data/Mesh.hpp>
#include <Engine/Data/ViewingParameters.hpp>
#include <Gui/Viewer/CameraManipulator.hpp>

#include "BezierUtils/CubicBezierApproximation.hpp"
#include "CurveEditor.hpp"

using namespace Ra::Core;
using namespace Ra::Core::Geometry;
using namespace Ra::Core::Utils;
using namespace Ra::Engine;
using namespace Ra::Engine::Scene;

CurveEditor::CurveEditor( const VectorArray<Curve2D::Vector>& polyline, MyViewer* viewer ) :
    Entity( "Curve Editor" ), m_viewer( viewer ) {

    m_entityMgr = RadiumEngine::getInstance()->getEntityManager();
    m_roMgr     = RadiumEngine::getInstance()->getRenderObjectManager();

    // Approximate polyline with bezier
    CubicBezierApproximation approximator;
    approximator.init( polyline );
    approximator.compute();
    auto solution    = approximator.getSolution();
    auto solutionPts = solution.getCtrlPoints();

    // Create and render solution entities
    CurveFactory factory;
    std::vector<Vector3Array> allCtrlPts;
    m_viewer->makeCurrent();
    for ( unsigned int i = 0; i < solutionPts.size() - 3; i += 3 ) {
        Vector3Array ctrlPts { Vector3( solutionPts[i].x(), 0, solutionPts[i].y() ),
                               Vector3( solutionPts[i + 1].x(), 0, solutionPts[i + 1].y() ),
                               Vector3( solutionPts[i + 2].x(), 0, solutionPts[i + 2].y() ),
                               Vector3( solutionPts[i + 3].x(), 0, solutionPts[i + 3].y() ) };
        auto e = factory.createCurveComponent( this, ctrlPts, i / 3 );
        m_curveEntities.push_back( e );
        allCtrlPts.push_back( ctrlPts );
    }

    auto e = PointFactory::createPointComponent( this, allCtrlPts[0][0], { 0 }, 0, Color::Blue() );
    m_pointEntities.push_back( e );

    int nameIndex = 1;
    for ( unsigned int i = 0; i < allCtrlPts.size(); i++ ) {
        for ( unsigned int j = 1; j < allCtrlPts[i].size() - 1; j++ ) {
            e = PointFactory::createPointComponent( this, allCtrlPts[i][j], { i }, nameIndex );
            m_pointEntities.push_back( e );
            nameIndex++;
        }
        if ( i == allCtrlPts.size() - 1 )
            e = PointFactory::createPointComponent(
                this, allCtrlPts[i][3], { i }, nameIndex, Color::Blue() );
        else
            e = PointFactory::createPointComponent(
                this, allCtrlPts[i][3], { i, i + 1 }, nameIndex, Color::Blue() );
        m_pointEntities.push_back( e );
        nameIndex++;
    }
    m_viewer->doneCurrent();
    m_viewer->getRenderer()->buildAllRenderTechniques();
    m_viewer->needUpdate();
}

CurveEditor::~CurveEditor() {}

unsigned int CurveEditor::getPointIndex( unsigned int curveIdSize,
                                         unsigned int currentPtIndex,
                                         const Vector3& firstCtrlPt,
                                         const Vector3& currentPt ) {
    unsigned int pointIndex;
    if ( curveIdSize > 1 || currentPtIndex == m_pointEntities.size() - 1 ) {
        pointIndex = ( firstCtrlPt == currentPt ) ? 0 : 3;
    }
    else { pointIndex = ( currentPtIndex % 3 ); }
    return pointIndex;
}

void CurveEditor::updateCurve( unsigned int curveId,
                               unsigned int curveIdSize,
                               PointComponent* pointComponent,
                               const Vector3& oldPoint,
                               unsigned int currentPoint ) {
    auto component = m_curveEntities[curveId];
    auto ctrlPts   = component->m_ctrlPts;
    int pointIndex = getPointIndex( curveIdSize, currentPoint, ctrlPts[0], oldPoint );

    auto ro = m_roMgr->getRenderObject( m_pointEntities[currentPoint]->getRenderObjects()[0] );
    auto transform          = ro->getTransform();
    ctrlPts[pointIndex]     = transform * pointComponent->m_defaultPoint;
    pointComponent->m_point = ctrlPts[pointIndex];

    std::string name = m_curveEntities[curveId]->getName();
    removeComponent( name );
    auto e                   = CurveFactory::createCurveComponent( this, ctrlPts, name );
    m_curveEntities[curveId] = e;
}

void CurveEditor::refreshPoint( unsigned int pointIndex, const Vector3& newPoint ) {
    auto pointName      = m_pointEntities[pointIndex]->getName();
    auto pointComponent = m_pointEntities[pointIndex];
    auto pointColor     = pointComponent->m_color;
    auto pointCurve     = pointComponent->m_curveId;
    auto pointState     = pointComponent->m_state;
    Vector3 point;
    if ( newPoint == Vector3::Zero() )
        point = pointComponent->m_point;
    else
        point = newPoint;
    removeComponent( m_pointEntities[pointIndex]->getName() );
    auto pointEntity =
        PointFactory::createPointComponent( this, point, pointCurve, pointName, pointColor );
    pointEntity->m_state        = pointState;
    m_pointEntities[pointIndex] = pointEntity;
}

void CurveEditor::updateCurves( bool onRelease ) {

    if ( m_currentPoint < 0 ) return;

    auto pointComponent      = m_pointEntities[m_currentPoint];
    auto oldPoint            = pointComponent->m_point;
    unsigned int curveIdSize = pointComponent->m_curveId.size();

    m_viewer->makeCurrent();

    for ( unsigned int i = 0; i < curveIdSize; i++ ) {
        auto curveId = pointComponent->m_curveId[i];
        updateCurve( curveId, curveIdSize, pointComponent, oldPoint, m_currentPoint );
    }

    for ( unsigned int i = 0; i < m_tangentPoints.size(); i++ ) {
        auto tangentPtComponent = m_pointEntities[m_tangentPoints[i]];
        Vector3 tangentPt       = tangentPtComponent->m_point;
        auto symCurveId         = tangentPtComponent->m_curveId[0];
        updateCurve( symCurveId, 1, tangentPtComponent, tangentPt, m_tangentPoints[i] );
    }

    // Necessary for transform re-initialization
    if ( onRelease ) {
        refreshPoint( m_currentPoint );
        for ( unsigned int i = 0; i < m_tangentPoints.size(); i++ ) {
            refreshPoint( m_tangentPoints[i] );
        }
        m_tangentPoints.clear();
    }

    m_viewer->doneCurrent();
    m_viewer->getRenderer()->buildAllRenderTechniques();
    m_viewer->needUpdate();
}

Transform CurveEditor::computePointTransform( PointComponent* pointCmp,
                                              PointComponent* midPointCmp,
                                              const Vector3& worldPos ) {
    auto transform = Transform::Identity();
    auto midPoint  = midPointCmp->m_point;
    Vector3 diff   = ( midPoint - worldPos );
    if ( midPointCmp->m_state == PointComponent::SYMETRIC ) {
        transform.translate( ( midPoint + diff ) - pointCmp->m_defaultPoint );
    }
    else {
        diff.normalize();
        auto actualdiff = diff * ( midPoint - pointCmp->m_point ).norm();
        transform.translate( ( midPoint + actualdiff ) - pointCmp->m_defaultPoint );
    }
    return transform;
}

inline float CurveEditor::distanceSquared( const Vector3f& pointA, const Vector3f& pointB ) {
    float dx = pointA.x() - pointB.x();
    float dy = pointA.y() - pointB.y();
    float dz = pointA.z() - pointB.z();
    return dx * dx + dy * dy + dz * dz;
}

void CurveEditor::subdivisionBezier( int vertexIndex,
                                     unsigned int curveIndex,
                                     const Vector3Array& ctrlPts ) {

    auto bezier       = Geometry::CubicBezier( Vector2( ctrlPts[0].x(), ctrlPts[0].z() ),
                                         Vector2( ctrlPts[1].x(), ctrlPts[1].z() ),
                                         Vector2( ctrlPts[2].x(), ctrlPts[2].z() ),
                                         Vector2( ctrlPts[3].x(), ctrlPts[3].z() ) );
    float u           = float( vertexIndex ) / 100.f;
    Vector2 fu        = bezier.f( u );
    auto clickedPoint = Vector3( fu.x(), 0, fu.y() );

    // De Casteljau
    Vector3 firstPoint  = Math::linearInterpolate( ctrlPts[0], ctrlPts[1], u );
    Vector3 sndPoint    = Math::linearInterpolate( ctrlPts[1], ctrlPts[2], u );
    Vector3 thirdPoint  = Math::linearInterpolate( ctrlPts[2], ctrlPts[3], u );
    Vector3 fourthPoint = Math::linearInterpolate( firstPoint, sndPoint, u );
    Vector3 fifthPoint  = Math::linearInterpolate( sndPoint, thirdPoint, u );

    auto newCtrlPts = ctrlPts;
    newCtrlPts[1]   = firstPoint;
    newCtrlPts[2]   = fourthPoint;
    newCtrlPts[3]   = clickedPoint;

    auto nameCurve = m_curveEntities[curveIndex]->getName();
    removeComponent( nameCurve );
    m_curveEntities[curveIndex] = CurveFactory::createCurveComponent( this, newCtrlPts, nameCurve );

    refreshPoint( curveIndex * 3 + 1, firstPoint );
    refreshPoint( curveIndex * 3 + 2, thirdPoint );

    auto firstInsertionIdx = curveIndex * 3 + 2;
    auto ptE               = PointFactory::createPointComponent(
        this, clickedPoint, { curveIndex, curveIndex + 1 }, m_pointEntities.size(), Color::Blue() );
    m_pointEntities.insert( m_pointEntities.begin() + firstInsertionIdx, ptE );

    ptE = PointFactory::createPointComponent(
        this, fourthPoint, { curveIndex }, m_pointEntities.size() );
    m_pointEntities.insert( m_pointEntities.begin() + firstInsertionIdx, ptE );

    ptE = PointFactory::createPointComponent(
        this, fifthPoint, { curveIndex + 1 }, m_pointEntities.size() );
    m_pointEntities.insert( m_pointEntities.begin() + ( ( curveIndex + 1 ) * 3 + 1 ), ptE );

    Vector3Array newCtrlPts1;
    newCtrlPts1.push_back( clickedPoint );
    newCtrlPts1.push_back( fifthPoint );
    newCtrlPts1.push_back( thirdPoint );
    newCtrlPts1.push_back( ctrlPts[3] );

    auto curveE = CurveFactory::createCurveComponent( this, newCtrlPts1, m_curveEntities.size() );
    m_curveEntities.insert( m_curveEntities.begin() + curveIndex + 1, curveE );

    for ( unsigned int i = ( ( curveIndex + 1 ) * 3 + 2 ); i < m_pointEntities.size(); i++ ) {
        auto ptCmp = m_pointEntities[i];
        for ( unsigned int j = 0; j < ptCmp->m_curveId.size(); j++ ) {
            ptCmp->m_curveId[j] += 1;
        }
    }
}

void CurveEditor::addPointAtEnd( const Vector3& worldPos ) {
    auto lastIndex  = m_pointEntities.size() - 1;
    auto beforeLast = m_pointEntities[lastIndex - 1];
    auto last       = m_pointEntities[lastIndex];
    Vector3 diff    = ( last->m_point - beforeLast->m_point );
    diff.normalize();

    Vector3 secondPt  = ( last->m_point + diff );
    Vector3 thirdDiff = ( last->m_point - worldPos );
    thirdDiff.normalize();
    Vector3 thirdPt = ( worldPos - diff );

    Vector3Array ctrlPts { last->m_point, secondPt, thirdPt, worldPos };

    auto e = CurveFactory::createCurveComponent( this, ctrlPts, m_curveEntities.size() );
    m_curveEntities.push_back( e );

    unsigned int pointIndex = lastIndex + 1;
    last->m_curveId.push_back( ( pointIndex / 3 ) );

    auto ptC =
        PointFactory::createPointComponent( this, ctrlPts[1], { ( pointIndex / 3 ) }, pointIndex );
    m_pointEntities.push_back( ptC );

    ptC = PointFactory::createPointComponent(
        this, ctrlPts[2], { ( pointIndex / 3 ) }, pointIndex + 1 );
    m_pointEntities.push_back( ptC );

    ptC = PointFactory::createPointComponent(
        this, ctrlPts[3], { ( pointIndex / 3 ) }, pointIndex + 2, Color::Blue() );
    m_pointEntities.push_back( ptC );
}

void CurveEditor::addPointInCurve( const Vector3& worldPos, int mouseX, int mouseY ) {
    auto camera = m_viewer->getCameraManipulator()->getCamera();
    auto radius = int( m_viewer->getRenderer()->getBrushRadius() );
    bool found  = false;
    Rendering::Renderer::PickingResult pres;

    for ( int i = mouseX - radius; i < mouseX + radius && !found; i++ ) {
        for ( int j = mouseY - radius; j < mouseY + radius; j++ ) {

            Rendering::Renderer::PickingQuery query { Vector2( i, m_viewer->height() - j ),
                                                      Rendering::Renderer::SELECTION,
                                                      Rendering::Renderer::RO };
            Data::ViewingParameters renderData {
                camera->getViewMatrix(), camera->getProjMatrix(), 0 };
            pres = m_viewer->getRenderer()->doPickingNow( query, renderData );

            if ( pres.getRoIdx().isValid() && m_roMgr->exists( pres.getRoIdx() ) ) {
                auto ro = m_roMgr->getRenderObject( pres.getRoIdx() );
                if ( ro->getMesh()->getNumVertices() == 2 ) continue;
                pres.removeDuplicatedIndices();
                auto e = ro->getComponent();

                int curveIndex = -1;
                for ( int z = 0; z < m_curveEntities.size(); z++ ) {
                    if ( m_curveEntities[z] == e ) {
                        curveIndex = z;
                        break;
                    }
                }
                if ( curveIndex < 0 ) continue;

                auto meshPtr  = ro->getMesh().get();
                auto mesh     = dynamic_cast<Data::Mesh*>( meshPtr );
                auto curveCmp = static_cast<CurveComponent*>( ro->getComponent() );
                auto ctrlPts  = curveCmp->m_ctrlPts;

                if ( mesh != nullptr ) {
                    float bestV = std::numeric_limits<float>::max();
                    int vIndex  = -1;
                    for ( unsigned int w = 0; w < mesh->getNumVertices(); w++ ) {
                        auto dist =
                            distanceSquared( worldPos, mesh->getCoreGeometry().vertices()[w] );
                        if ( dist < bestV ) {
                            bestV  = dist;
                            vIndex = w;
                        }
                    }

                    subdivisionBezier( vIndex, curveIndex, ctrlPts );
                    found = true;
                    break;
                }
            }
        }
    }
}

bool CurveEditor::processHover( std::shared_ptr<Rendering::RenderObject> ro ) {
    auto e = ro->getComponent();

    // if not a control point -> do nothing
    for ( int i = 0; i < m_pointEntities.size(); i++ ) {
        if ( e == m_pointEntities[i] ) {
            m_currentPoint = i;
            break;
        }
    }
    if ( m_currentPoint < 0 ) return false;

    if ( m_currentPoint % 3 == 0 )
        m_savedPoint = m_currentPoint;
    else if ( m_savedPoint >= 0 && m_savedPoint - 1 != m_currentPoint &&
              m_savedPoint + 1 != m_currentPoint )
        m_savedPoint = -1;

    ro->getMaterial()->getParameters().addParameter( "material.color", Color::Red() );
    m_selectedRo = ro;
    return true;
}

void CurveEditor::processUnhovering() {
    auto pointCmp = static_cast<PointComponent*>( m_selectedRo->getComponent() );
    m_selectedRo->getMaterial()->getParameters().addParameter( "material.color",
                                                               pointCmp->m_color );
    m_selectedRo   = nullptr;
    m_currentPoint = -1;
}

void CurveEditor::processPicking( const Vector3& worldPos ) {
    if ( m_currentPoint < 0 ) return;
    auto pointComponent = static_cast<PointComponent*>( m_selectedRo->getComponent() );
    auto point          = pointComponent->m_point;

    auto transformTranslate = Transform::Identity();
    transformTranslate.translate( worldPos - point );

    auto transform = m_selectedRo->getLocalTransform() * transformTranslate;
    m_selectedRo->setLocalTransform( transform );

    if ( !( m_currentPoint == 0 || m_currentPoint == 1 ||
            m_currentPoint == m_pointEntities.size() - 2 ||
            m_currentPoint == m_pointEntities.size() - 1 ) ) {

        if ( m_currentPoint % 3 == 0 ) {
            auto leftRo = m_roMgr->getRenderObject(
                m_pointEntities[m_currentPoint - 1]->getRenderObjects()[0] );
            auto leftTransform = leftRo->getTransform() * transformTranslate;
            leftRo->setLocalTransform( leftTransform );

            auto rightRo = m_roMgr->getRenderObject(
                m_pointEntities[m_currentPoint + 1]->getRenderObjects()[0] );
            auto rightTransform = rightRo->getTransform() * transformTranslate;
            rightRo->setLocalTransform( rightTransform );

            if ( m_tangentPoints.empty() ) {
                m_tangentPoints.push_back( m_currentPoint - 1 );
                m_tangentPoints.push_back( m_currentPoint + 1 );
            }
        }
        else if ( m_currentPoint % 3 == 2 ) {
            auto pointMid = m_pointEntities[m_currentPoint + 1];
            if ( !( pointMid->m_state == PointComponent::DEFAULT ) ) {
                pointComponent    = m_pointEntities[m_currentPoint + 2];
                auto symTransform = computePointTransform( pointComponent, pointMid, worldPos );
                auto ro = m_roMgr->getRenderObject( pointComponent->getRenderObjects()[0] );
                ro->setLocalTransform( symTransform );

                if ( m_tangentPoints.empty() ) { m_tangentPoints.push_back( m_currentPoint + 2 ); }
            }
        }
        else if ( m_currentPoint % 3 == 1 ) {
            auto pointMid = m_pointEntities[m_currentPoint - 1];
            if ( !( pointMid->m_state == PointComponent::DEFAULT ) ) {
                pointComponent    = m_pointEntities[m_currentPoint - 2];
                auto symTransform = computePointTransform( pointComponent, pointMid, worldPos );
                auto ro = m_roMgr->getRenderObject( pointComponent->getRenderObjects()[0] );
                ro->setLocalTransform( symTransform );

                if ( m_tangentPoints.empty() ) { m_tangentPoints.push_back( m_currentPoint - 2 ); }
            }
        }
    }
    updateCurves();
}

void CurveEditor::setSmooth( bool smooth ) {
    if ( m_savedPoint >= 0 ) {
        if ( smooth ) {
            m_pointEntities[m_savedPoint]->m_state = PointComponent::SMOOTH;
            smoothify( m_savedPoint );
        }
        else
            m_pointEntities[m_savedPoint]->m_state = PointComponent::DEFAULT;
    }
}

void CurveEditor::setSymetry( bool symetry ) {
    if ( m_savedPoint >= 0 ) {
        if ( symetry ) {
            m_pointEntities[m_savedPoint]->m_state = PointComponent::SYMETRIC;
            symmetrize( m_savedPoint );
        }
        else
            m_pointEntities[m_savedPoint]->m_state = PointComponent::SMOOTH;
    }
}

void CurveEditor::smoothify( int pointId ) {
    if ( ( pointId == 0 || pointId == 1 || pointId == m_pointEntities.size() - 2 ||
           pointId == m_pointEntities.size() - 1 ) )
        return;

    m_viewer->makeCurrent();
    Vector3 backPoint  = m_pointEntities[pointId - 1]->m_point;
    Vector3 back       = backPoint - m_pointEntities[pointId]->m_point;
    Vector3 frontPoint = m_pointEntities[pointId + 1]->m_point;
    Vector3 front      = frontPoint - m_pointEntities[pointId]->m_point;

    Vector3 newPointDir = back - front;
    newPointDir.normalize();
    newPointDir          = newPointDir * back.norm();
    Vector3 newBackPoint = m_pointEntities[pointId]->m_point + newPointDir;

    auto transform = Transform::Identity();
    transform.translate( newBackPoint - m_pointEntities[pointId - 1]->m_point );
    auto roIdx = m_pointEntities[pointId - 1]->getRenderObjects()[0];
    auto ro    = m_roMgr->getRenderObject( roIdx );
    ro->setLocalTransform( transform );
    m_pointEntities[pointId - 1]->m_point = newBackPoint;

    newPointDir.normalize();
    newPointDir           = newPointDir * front.norm();
    Vector3 newFrontPoint = m_pointEntities[pointId]->m_point - newPointDir;

    transform = Transform::Identity();
    transform.translate( newFrontPoint - m_pointEntities[pointId + 1]->m_point );
    roIdx = m_pointEntities[pointId + 1]->getRenderObjects()[0];
    ro    = m_roMgr->getRenderObject( roIdx );
    ro->setLocalTransform( transform );
    m_pointEntities[pointId + 1]->m_point = newFrontPoint;

    m_currentPoint = pointId - 1;
    updateCurves( true );
    m_currentPoint = pointId + 1;
    updateCurves( true );
    m_currentPoint = -1;

    m_viewer->doneCurrent();
    m_viewer->getRenderer()->buildAllRenderTechniques();
    m_viewer->needUpdate();
}

void CurveEditor::symmetrize( int pointId ) {
    if ( ( pointId == 0 || pointId == 1 || pointId == m_pointEntities.size() - 2 ||
           pointId == m_pointEntities.size() - 1 ) )
        return;

    m_viewer->makeCurrent();

    Vector3 backPoint  = m_pointEntities[pointId - 1]->m_point;
    Vector3 back       = backPoint - m_pointEntities[pointId]->m_point;
    Vector3 frontPoint = m_pointEntities[pointId + 1]->m_point;
    Vector3 front      = frontPoint - m_pointEntities[pointId]->m_point;

    auto transform = Transform::Identity();
    Index roIdx    = -1;
    if ( std::min( back.norm(), front.norm() ) == back.norm() ) {
        front.normalize();
        front            = front * back.norm();
        Vector3 newPoint = m_pointEntities[pointId]->m_point + front;
        transform.translate( newPoint - m_pointEntities[pointId + 1]->m_point );
        roIdx                                 = m_pointEntities[pointId + 1]->getRenderObjects()[0];
        m_pointEntities[pointId + 1]->m_point = newPoint;
        m_currentPoint                        = pointId + 1;
    }
    else {
        back.normalize();
        back             = back * front.norm();
        Vector3 newPoint = m_pointEntities[pointId]->m_point + back;
        transform.translate( newPoint - m_pointEntities[pointId - 1]->m_point );
        roIdx                                 = m_pointEntities[pointId - 1]->getRenderObjects()[0];
        m_pointEntities[pointId - 1]->m_point = newPoint;
        m_currentPoint                        = pointId - 1;
    }
    auto ro = m_roMgr->getRenderObject( roIdx );
    ro->setLocalTransform( transform );
    updateCurves( true );
    m_currentPoint = -1;

    m_viewer->doneCurrent();
    m_viewer->getRenderer()->buildAllRenderTechniques();
    m_viewer->needUpdate();
}
