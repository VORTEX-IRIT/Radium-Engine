#include "CurveFactory.hpp"
#include "Gui/MyViewer.hpp"
#include "PointFactory.hpp"
#include <Core/Geometry/Curve2D.hpp>
#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Rendering/RenderObjectManager.hpp>
#include <Engine/Rendering/Renderer.hpp>

class CurveEditor : public Ra::Engine::Scene::Entity
{

  public:
    CurveEditor( const Ra::Core::VectorArray<Ra::Core::Geometry::Curve2D::Vector>& polyline,
                 MyViewer* viewer );
    ~CurveEditor();

    /**
     * @brief add a point at the end of the polyline
     * @param the world position where to add the point
     */
    void addPointAtEnd( const Ra::Core::Vector3& worldPos );

    /**
     * @brief add a point in the existing polyline
     * @param the world position where to add the point
     * @param the corresponding window x position
     * @param the corresponding window y position
     */
    void addPointInCurve( const Ra::Core::Vector3& worldPos, int mouseX, int mouseY );

    /**
     * @brief update all curves, if onRelease then it will clean the entities transform
     * @param the world position where to add the point
     */
    void updateCurves( bool onRelease = false );

    /**
     * @brief change the color of the corresponding ro and save it for future manipulation
     * @param the renderObject that is currently being hovered
     */
    bool processHover( std::shared_ptr<Ra::Engine::Rendering::RenderObject> ro );

    /**
     * @brief change saved ro color to their default and unsave the ro
     */
    void processUnhovering();

    /**
     * @brief process the saved ro and move it to the given world position
     * @param the world position of where to move the saved ro
     */
    void processPicking( const Ra::Core::Vector3& worldPos );

    std::shared_ptr<Ra::Engine::Rendering::RenderObject> getSelected() { return m_selectedRo; }

    void resetSelected() {
        m_selectedRo   = nullptr;
        m_currentPoint = -1;
    }

    PointComponent* getSavedPoint() {
        if ( m_savedPoint >= 0 )
            return m_pointEntities[m_savedPoint];
        else
            return nullptr;
    }

    /**
     * @brief smooth to keep a G1 continuity
     * @param smooth state
     */
    void setSmooth( bool smooth );

    /**
     * @brief symetry to keep a C1 continuity
     * @param symetry state
     */
    void setSymetry( bool symetry );

  private:
    inline float distanceSquared( const Ra::Core::Vector3f& pointA,
                                  const Ra::Core::Vector3f& pointB );

    unsigned int getPointIndex( unsigned int curveIdSize,
                                unsigned int currentPtIndex,
                                const Ra::Core::Vector3& firstCtrlPt,
                                const Ra::Core::Vector3& currentPt );

    void updateCurve( unsigned int curveId,
                      unsigned int curveIdSize,
                      PointComponent* pointComponent,
                      const Ra::Core::Vector3& currentPt,
                      unsigned int currentPoint );

    Ra::Core::Transform computePointTransform( PointComponent* pointCmp,
                                               PointComponent* midPoint,
                                               const Ra::Core::Vector3& worldPos );
    void subdivisionBezier( int vertexIndex,
                            unsigned int curveIndex,
                            const Ra::Core::Vector3Array& ctrlPts );
    void refreshPoint( unsigned int pointIndex,
                       const Ra::Core::Vector3& newPoint = Ra::Core::Vector3::Zero() );

    void smoothify( int pointId );

    void symmetrize( int pointId );

  private:
    Ra::Engine::Scene::EntityManager* m_entityMgr;
    int m_currentPoint { -1 };
    std::shared_ptr<Ra::Engine::Rendering::RenderObject> m_selectedRo { nullptr };
    std::vector<PointComponent*> m_pointEntities;
    std::vector<CurveComponent*> m_curveEntities;
    std::vector<unsigned int> m_tangentPoints;
    Ra::Engine::Rendering::RenderObjectManager* m_roMgr;
    int m_savedPoint { -1 };

    MyViewer* m_viewer { nullptr };
};
