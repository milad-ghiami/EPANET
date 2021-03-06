//
// test_epanet_toolkit.cpp
//
// Date Created: January 24, 2018
//
// Author: Michael E. Tryby
//         US EPA - ORD/NRMRL
//

//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <string>
#include "epanet2.h"

// NOTE: Project Home needs to be updated to run unit test
#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

using namespace std;


BOOST_AUTO_TEST_SUITE (test_toolkit)

BOOST_AUTO_TEST_CASE (test_alloc_free)
{
    int error = 0;
    EN_ProjectHandle ph = NULL;

    error = EN_createproject(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph != NULL);

    error = EN_deleteproject(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph == NULL);
}

BOOST_AUTO_TEST_CASE (test_open_close)
{
    EN_ProjectHandle ph = NULL;
    EN_createproject(&ph);

    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);

    int error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_CASE(test_epanet)
{
    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);

    int error = ENepanet(path_inp.c_str(), path_rpt.c_str(), path_out.c_str(), NULL);
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_SUITE_END()


struct Fixture{
    Fixture() {
        path_inp = std::string(DATA_PATH_INP);
        path_rpt = std::string(DATA_PATH_RPT);
        path_out = std::string(DATA_PATH_OUT);

        EN_createproject(&ph);
        error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());

    }

    ~Fixture() {
      error = EN_close(ph);
      EN_deleteproject(&ph);
  }

  std::string path_inp;
  std::string path_rpt;
  std::string path_out;

  int error;
  EN_ProjectHandle ph;

};

BOOST_AUTO_TEST_SUITE(test_epanet_fixture)

BOOST_FIXTURE_TEST_CASE(test_epanet, Fixture)
{
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_report(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_hyd_step, Fixture)
{
    int flag = 00;
    long t, tstep;

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);

    } while (tstep > 0);
    
    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_qual_step, Fixture)
{
    int flag = 0;
    long t, tstep;

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_openQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initQ(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runQ(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_nextQ(ph, &tstep);
        BOOST_REQUIRE(error == 0);

    } while (tstep > 0);
    
    error = EN_closeQ(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_progressive_stepping, Fixture)
{
    int flag = EN_NOSAVE;
    long t, tstep_h, tstep_q;

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    error = EN_openQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initQ(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {


        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_runQ(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_nextH(ph, &tstep_h);
        BOOST_REQUIRE(error == 0);

        error = EN_nextQ(ph, &tstep_q);
        BOOST_REQUIRE(error == 0);

    } while (tstep_h > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_closeQ(ph);
    BOOST_REQUIRE(error == 0);

}

BOOST_FIXTURE_TEST_CASE(test_setdemandpattern, Fixture)
{
    int i, j, pat_index, pat_index_2, numDemands, nnodes;
	char newpat[] = "new_pattern";

	// get the number of nodes
    error = EN_getcount(ph, EN_NODECOUNT, &nnodes);
    BOOST_REQUIRE(error == 0);
	
	// add a new pattern
    error = EN_addpattern(ph, newpat);
	BOOST_REQUIRE(error == 0);

	// get the new patterns index, should be as the number of patterns
    error = EN_getpatternindex(ph, newpat, &pat_index);
	BOOST_REQUIRE(error == 0);
	
	for (i = 1; i <= nnodes; i++) {
		// get the number of demand categories
		error = EN_getnumdemands(ph, i, &numDemands);
		BOOST_REQUIRE(error == 0);
		
		for (j = 1; j <= numDemands; j++) {
			// set demand patterns
			error = EN_setdemandpattern(ph, i, j, pat_index); 
			BOOST_REQUIRE(error == 0);
			// get demand patterns should be the same with set
			error = EN_getdemandpattern(ph, i, j, &pat_index_2); 
			BOOST_REQUIRE(error == 0);
			BOOST_REQUIRE(pat_index == pat_index_2);
		}
	}
}
BOOST_FIXTURE_TEST_CASE(test_addpattern, Fixture)
{
    int pat_index, n_patterns_1, n_patterns_2;
    char newpat[] = "new_pattern";
    
    // get the number of current patterns
    error = EN_getcount(ph, EN_PATCOUNT, &n_patterns_1);
    BOOST_REQUIRE(error == 0);
    
    // add a new pattern
    error = EN_addpattern(ph, newpat);
    BOOST_REQUIRE(error == 0);

    // get the new patterns count, shoul dbe the old one + 1
    error = EN_getcount(ph, EN_PATCOUNT, &n_patterns_2);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(n_patterns_1 + 1 == n_patterns_2);
    
    // gwt the new patterns index, should be as the number of patterns
    error = EN_getpatternindex(ph, newpat, &pat_index);
    BOOST_REQUIRE(pat_index == n_patterns_2);
}

BOOST_AUTO_TEST_SUITE_END()
