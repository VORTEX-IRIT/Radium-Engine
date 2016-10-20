#include <Core/Mesh/DCEL/Iterator/Edge/EFIterator.hpp>

#include <Core/Mesh/DCEL/HalfEdge.hpp>
#include <Core/Mesh/DCEL/Face.hpp>

namespace Ra {
namespace Core {



/// CONSTRUCTOR
EFIterator::EFIterator( HalfEdge_ptr& he ) : EIterator< Face >( he )
{
}



/// DESTRUCTOR
EFIterator::~EFIterator() { }



/// LIST
inline FaceList EFIterator::list() const
{
    FaceList L;
    HalfEdge_ptr h1 = m_he;
    HalfEdge_ptr h2 = h1->Twin();
    Face_ptr f1, f2;

    if (h1 != NULL)
    {
        f1 = h1->F();
        L.push_back(f1);
        h1 = h1->Next()->Twin();
    }
    if (h2 != NULL)
    {
        f2 = h2->F();
        L.push_back(f2);
        h2 = h2->Prev()->Twin();
    }

    // First path around v2
    while(h1 != NULL || h2 != NULL)
    {
        if (h1 != NULL)
        {
            f1 = h1->F();
            if (f1 == f2) break;
            L.push_back(f1);
            h1 = h1->Next()->Twin();
        }
        if (h2 != NULL)
        {
            f2 = h2->F();
            if (f2 == f1) break;
            L.push_back(f2);
            h2 = h2->Prev()->Twin();
        }
    }

    // Second path around v1
    h1 = m_he;
    if (h1->Twin() != NULL) h2 = h1->Twin()->Next()->Twin();
    else h2 = nullptr;
    h1 = h1->Prev()->Twin();
    while(h1 != NULL || h2 != NULL)
    {
        if (h1 != NULL)
        {
            f1 = h1->F();
            if (f1 == f2) break;
            L.push_back(f1);
            h1 = h1->Prev()->Twin();
        }
        if (h2 != NULL)
        {
            f2 = h2->F();
            if (f2 == f1) break;
            L.push_back(f2);
            h2 = h2->Next()->Twin();
        }
    }

    return L;
}



/// OPERATOR
inline Face* EFIterator::operator->() const
{
    return m_he->F().get();
}



} // namespace Core
} // namespace Ra
