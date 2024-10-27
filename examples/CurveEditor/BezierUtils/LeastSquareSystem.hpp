#pragma once

#include <Core/Utils/Log.hpp>
#include <Eigen/Dense>
#include <Eigen/Eigen>
#include <Eigen/Sparse>
#include <algorithm>
#include <map>

using Eigen::MatrixXf;
using Eigen::SparseMatrix;
using Eigen::Triplet;
using Eigen::VectorXf;

/**
 * @brief The LeastSquareSystem class allows
 * setting up and solving an n-dimensional least square system
 * of the form Ax=b.
 */
class LeastSquareSystem
{
  public:
    using CoefTriplet = Triplet<float>;
    using SparseMat   = SparseMatrix<float, Eigen::ColMajor>;

    /**
     * @brief LeastSquareSystem of the dimension given by the parameters
     * @param number of variables of the system
     * @param dimension of the variables (optional)
     * @param expected number of constraints (optional)
     */
    explicit LeastSquareSystem( int nvar, int dim = 1, int exp_ncstr = 0 ) :
        m_nvar( nvar ), m_dim( dim ) {
        m_constrainedVar.resize( m_nvar, false );
        if ( exp_ncstr > 0 ) { m_bval.reserve( exp_ncstr ); }
    }

    /**
     * @brief Adding a constraint to the system
     * @param coefs of the corresponding row in the matrix A,
     *        only non zero coefs will be stored in the sparse matrix
     *        coefs are 1D : same for all dimension of the variable
     * @param value of the corresponding row in b (n-dimensional)
     * @param weight of the constraint (optional)
     * @return 0 if everything went ok, -1 otherwise (the constraint is not added in this case)
     */
    int addConstraint( std::map<int, float> coefs, VectorXf res, float w = 1 ) {
        using namespace Ra::Core::Utils;
        if ( coefs.empty() || ( res.rows() != m_dim ) ) {
            LOG( logERROR ) << "Constraint has wrong dimension";
            return -1;
        }

        bool anyNonZeroCoef { false };

        for ( const auto& [id_var, coef] : coefs ) {
            if ( w * fabs( coef ) < 1e-8 ) { continue; }
            anyNonZeroCoef = true;
            for ( int d = 0; d < m_dim; ++d ) {
                m_tripletList.emplace_back(
                    CoefTriplet( m_dim * m_ncstr + d, m_dim * id_var + d, w * coef ) );
            }
            m_constrainedVar[id_var] = true;
        }

        if ( anyNonZeroCoef ) {
            m_bval.push_back( w * res );
            ++m_ncstr;
            return 0;
        }

        return -1;
    }

    /**
     * @brief Call this once all constraints have been added to the system.
     *        Sets up and solves the LSS if it is solvable.
     * @param writes the solution in a (nvar x ndim) matrix (a row = a variable)
     * @return true if everything went ok, false if the system is unsolvable or badly built
     */
    bool solve( MatrixXf& solution ) const {
        using namespace Ra::Core::Utils;

        if ( !isSolvable() ) {
            LOG( logERROR ) << "Least square system unsolvable";
            return false;
        }

        SparseMat A;
        VectorXf b;
        bool setupFlag { this->setUpSystem( A, b ) };
        if ( !setupFlag ) {
            LOG( logERROR ) << "Error while setting up least square system";
            return false;
        }

        VectorXf flat_solution;
        bool solveFlag { jacobiSVD( A, b, flat_solution ) };
        if ( !solveFlag ) {
            LOG( logERROR ) << "Error while solving the least square system";
            return false;
        }

        solution = flat_solution.reshaped<Eigen::RowMajor>( m_nvar, m_dim );

        return true;
    }

    /**
     * @brief Prints details of the current state of the system
     * @param output log level (optional)
     */
    void log( Ra::Core::Utils::TLogLevel logL = Ra::Core::Utils::logINFO ) const {
        using namespace Ra::Core::Utils;

        LOG( logL ) << "LSS with " << m_nvar << " variables of dimension " << m_dim << ", and "
                    << m_ncstr << " constraints";

        LOG( logL ) << "Triplets (" << m_tripletList.size() << ") : ";
        for ( const auto& t : m_tripletList ) {
            LOG( logL ) << "(" << t.row() << "," << t.col() << ") = " << t.value();
        }

        LOG( logL ) << "Values (" << m_bval.size() << ")";
        for ( const auto& b : m_bval ) {
            LOG( logL ) << b.transpose();
        }

        std::stringstream logss;
        logss << "Constrained variables :";
        int v = -1;
        for ( const auto& c : m_constrainedVar ) {
            logss << "(" << ++v << ": " << c << ") ";
        }
        LOG( logL ) << logss.str();
        logss.str( "" );
    }

  private:
    int m_nvar { 0 };  // number of variables in the system
    int m_dim { 1 };   // dimension of the variables (n)
    int m_ncstr { 0 }; // number of constraints in the system

    std::vector<bool> m_constrainedVar;   // constrained variables
    std::list<CoefTriplet> m_tripletList; // coefs of the sparse A matrix
    std::vector<VectorXf> m_bval;         // values of the b vector (each coef is n-dimensional)

    /**
     * @brief Setting up the matrices of the system
     * @param output A sparse (nconstr*ndim) x (nvar*ndim) matrix
     * @param output b (nconstr*ndim) vector
     * @return true if the system was built properly, false otherwise
     */
    bool setUpSystem( SparseMat& A, VectorXf& b ) const {
        if ( ( m_ncstr != (int)m_bval.size() ) ) { return false; }

        A.resize( m_ncstr * m_dim, m_nvar * m_dim );
        A.setFromTriplets( m_tripletList.begin(), m_tripletList.end() );
        A.makeCompressed();

        b.resize( m_ncstr * m_dim );
        for ( int c = 0; c < m_ncstr; ++c ) {
            for ( int d = 0; d < m_dim; ++d ) {
                b( m_dim * c + d ) = m_bval[c]( d );
            }
        }
        return true;
    }

    /**
     * @brief Check if system is solvable.
     *        Only check if all variable are constrained and if more constraints than variables
     *        Does not guarantee the rank of the A matrix will be sufficient
     * @return true if solvable, false otherwise
     */
    bool isSolvable() const {
        using namespace Ra::Core::Utils;
        if ( m_ncstr < m_nvar ) {
            LOG( logERROR ) << "LSS - Underconstrained problem";
            return false;
        }

        auto unconstr = std::find( m_constrainedVar.cbegin(), m_constrainedVar.cend(), false );
        if ( unconstr != m_constrainedVar.cend() ) {
            LOG( logERROR ) << "LSS - Unconstrained variable(s)";
            return false;
        }

        return true;
    }

    bool jacobiSVD( const SparseMat& A, const VectorXf& b, VectorXf& x ) const {
        MatrixXf denseA( A );
        Eigen::JacobiSVD<MatrixXf> svd( denseA, Eigen::ComputeThinU | Eigen::ComputeThinV );
        x = svd.solve( b );
        return true;
    }
};
