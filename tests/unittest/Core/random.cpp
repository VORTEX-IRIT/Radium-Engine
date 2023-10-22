#include <catch2/catch.hpp>
#include <unittestUtils.hpp>

#include <iomanip>
#include <iostream>

#include <Core/Random/RandomPointSet.hpp>
using namespace Ra::Core::Random;

TEST_CASE( "Core/Random/RandomPointSet", "[Core][Core/Random][PointSet]" ) {
    SECTION( "Fibonacci sequence" ) {
        std::array<Scalar, 5> fib_verif { 0_ra,
                                          0.61803398874989479150343640867504_ra,
                                          1.2360679774997895830068728173501_ra,
                                          1.8541019662496844855326116885408_ra,
                                          2.4721359549995791660137456347002_ra };
        FibonacciSequence fib { 2 };
        // Our fibonacci sequence is only defined for more than 5 points
        REQUIRE( fib.range() == 5 );
        for ( size_t i = 0; i < 5; ++i ) {
            REQUIRE( isApprox( fib( i ), fib_verif[i] ) );
        }
    }

    SECTION( "VanDerCorput sequence" ) {
        std::array<Scalar, 5> vdc_verif { 0_ra, 0.5_ra, 0.25_ra, 0.75_ra, 0.125_ra };
        VanDerCorputSequence vdc;
        for ( size_t i = 0; i < 5; ++i ) {
            REQUIRE( isApprox( vdc( i ), vdc_verif[i] ) );
        }
    }

    SECTION( "Fibonacci point set" ) {
        std::array<std::pair<Scalar, Scalar>, 5> fibseq_verif {
            std::pair<Scalar, Scalar> { 0_ra, 0_ra / 5_ra },
            { 0.61803398874989479150343640867504_ra, 1_ra / 5_ra },
            { 1.2360679774997895830068728173501_ra, 2_ra / 5_ra },
            { 1.8541019662496844855326116885408_ra, 3_ra / 5_ra },
            { 2.4721359549995791660137456347002_ra, 4_ra / 5_ra } };
        FibonacciPointSet fibs { 5 };
        for ( size_t i = 0; i < 5; ++i ) {
            auto v = fibs( i );
            REQUIRE( isApprox( v[0], fibseq_verif[i].first ) );
            REQUIRE( isApprox( v[1], fibseq_verif[i].second ) );
        }
    }

    SECTION( "Hammersley point set" ) {
        std::array<std::pair<Scalar, Scalar>, 5> seq_verif {
            std::pair<Scalar, Scalar> { 0_ra, 0_ra / 5_ra },
            { 1_ra / 5_ra, 0.5_ra },
            { 2_ra / 5_ra, 0.25_ra },
            { 3_ra / 5_ra, 0.75_ra },
            { 4_ra / 5_ra, 0.125_ra } };
        HammersleyPointSet seq { 5 };
        for ( size_t i = 0; i < 5; ++i ) {
            auto v = seq( i );
            REQUIRE( isApprox( v[0], seq_verif[i].first ) );
            REQUIRE( isApprox( v[1], seq_verif[i].second ) );
        }
    }

    // todo, verify if the sequence is always the same (it should) on any systems/run
    SECTION( "MersenneTwister point set" ) {
#ifdef CORE_USE_DOUBLE
        // Sequence valid only when Scalar == double
        std::array<std::pair<Scalar, Scalar>, 5> seq_verif {
            std::pair<Scalar, Scalar> { 0.59284461651668263204584263803554_ra,
                                        0.84426574425659828282419994138763_ra },
            { 0.85794561998982987738315841852454_ra, 0.84725173738433123826752080276492_ra },
            { 0.62356369649610832173181051985011_ra, 0.38438170837375662536317122430773_ra },
            { 0.29753460535723419422282631785492_ra, 0.056712975933163663200264892338964_ra },
            { 0.27265629474158931122573790162278_ra, 0.47766511174464632016878340436961_ra } };
#else
        // Sequence valid only when Scalar == float
        std::array<std::pair<Scalar, Scalar>, 5> seq_verif {
            std::pair<Scalar, Scalar> { 0.548813521862030029296875_ra,
                                        0.59284460544586181640625_ra },
            { 0.71518933773040771484375_ra, 0.844265758991241455078125_ra },
            { 0.602763354778289794921875_ra, 0.857945621013641357421875_ra },
            { 0.544883191585540771484375_ra, 0.847251713275909423828125_ra },
            { 0.4236547946929931640625_ra, 0.623563706874847412109375_ra } };
#endif
        MersenneTwisterPointSet seq { 5 };
        for ( size_t i = 0; i < 5; ++i ) {
            auto v = seq( i );
            REQUIRE( isApprox( v[0], seq_verif[i].first ) );
            REQUIRE( isApprox( v[1], seq_verif[i].second ) );
        }
    }

    SECTION( "SphericalPointSet point set (Hammersley)" ) {
        std::array<std::pair<Scalar, Scalar>, 5> seq_verif {
            std::pair<Scalar, Scalar> { 1.2246467991473532071737640294584e-16_ra, 0_ra },
            { 0.30901699437494745126286943559535_ra, 0.95105651629515353118193843329209_ra },
            { -0.70062926922203661028731858095853_ra, 0.50903696045512725198989301134134_ra },
            { -0.70062926922203672130962104347418_ra, -0.50903696045512702994528808631003_ra },
            { 0.20439552950218897731105016646325_ra, -0.62906475622110624712490789534058_ra } };
        SphericalPointSet<HammersleyPointSet> seq { 5 };
        for ( size_t i = 0; i < 5; ++i ) {
            auto v = seq( i );
            REQUIRE( isApprox( v[0], seq_verif[i].first ) );
            REQUIRE( isApprox( v[1], seq_verif[i].second ) );
        }
    }
}
