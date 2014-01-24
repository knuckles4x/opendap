// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#include <sstream>

#include "config.h"

//#define DODS_DEBUG2
//#define DODS_DEBUG

#include "Byte.h"
#include "Int16.h"
#include "UInt16.h"
#include "Int32.h"
#include "UInt32.h"
#include "Float32.h"
#include "Float64.h"
#include "Str.h"
#include "Url.h"
#include "Array.h"
#include "Structure.h"
#include "Sequence.h"
#include "Grid.h"

#include "DDS.h"

#include "GNURegex.h"
#include "util.h"
#include "debug.h"

#include "testFile.h"
#include "test_config.h"

using namespace CppUnit;
using namespace std;

namespace libdap {

class DDSTest: public TestFixture {
private:
    DDS *dds1, *dds2;
    BaseTypeFactory factory;

    DDS *dds_dap4;

public:
    DDSTest() {
    }
    ~DDSTest() {
    }

    void setUp() {
        dds1 = new DDS(&factory, "test1");
        dds2 = new DDS(&factory, "test2");

        dds_dap4 = new DDS(&factory, "test2", "4.0");
    }

    void tearDown() {
        delete dds1; dds1 = 0;
        delete dds2; dds2 = 0;

        delete dds_dap4; dds_dap4 = 0;
    }

    bool re_match(Regex &r, const string &s) {
        int match = r.match(s.c_str(), s.length());
        DBG(cerr << "Match: " << match << " should be: " << s.length()
                << endl);
        return match == static_cast<int> (s.length());
    }

    // The tests commented exercise features no longer supported
    // by libdap. In particular, a DAS must now be properly structured
    // to work with transfer_attributes() - if a handler builds a malformed
    // DAS, it will need to specialize the BaseType::transfer_attributes()
    // method.
    CPPUNIT_TEST_SUITE( DDSTest );
#if 1
        CPPUNIT_TEST(transfer_attributes_test_1);
        CPPUNIT_TEST(transfer_attributes_test_2);

        CPPUNIT_TEST(symbol_name_test);
#endif
        // These test both transfer_attributes() and print_xml()
        CPPUNIT_TEST(print_xml_test);
#if 1
        CPPUNIT_TEST(print_xml_test2);
        CPPUNIT_TEST(print_xml_test3);

        // The X_1 tests look at the proper merging of hdf4's _dim_n attributes.
        // But that functionality was moved from libdap to the hdf4 handler.
        // CPPUNIT_TEST(print_xml_test3_1);

        CPPUNIT_TEST(print_xml_test4);
        CPPUNIT_TEST(print_xml_test5);
        // CPPUNIT_TEST(print_xml_test5_1);
        CPPUNIT_TEST(print_xml_test6);
        // CPPUNIT_TEST(print_xml_test6_1);
        CPPUNIT_TEST(print_dmr_test);

        CPPUNIT_TEST(get_response_size_test);
        CPPUNIT_TEST(get_response_size_test_c);
        CPPUNIT_TEST(get_response_size_test_c2);
        CPPUNIT_TEST(get_response_size_test_c3);
        CPPUNIT_TEST(get_response_size_test_seq);
        CPPUNIT_TEST(get_response_size_test_seq_c);
#endif

    CPPUNIT_TEST_SUITE_END();

    void transfer_attributes_test_1() {
        try {
            dds1->parse((string) TEST_SRC_DIR + "/dds-testsuite/fnoc1.nc.dds");
            DAS das;
            das.parse((string) TEST_SRC_DIR + "/dds-testsuite/fnoc1.nc.das");
            dds1->transfer_attributes(&das);

            DBG2(dds1->print_xml(cerr, false, ""));

            AttrTable &at = dds1->get_attr_table();
            AttrTable::Attr_iter i = at.attr_begin();
            CPPUNIT_ASSERT(i != at.attr_end() && at.get_name(i) == "NC_GLOBAL");
            CPPUNIT_ASSERT(i != at.attr_end() && at.get_name(++i) == "DODS_EXTRA");
        } catch (Error &e) {
            cout << "Error: " << e.get_error_message() << endl;
            CPPUNIT_FAIL("Error thrown!");
        }
    }

    void transfer_attributes_test_2() {
        try {
            dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/3B42.980909.5.HDF.dds");
            DAS das;
            das.parse((string) TEST_SRC_DIR + "/dds-testsuite/3B42.980909.5.hacked.HDF.das");
            dds2->transfer_attributes(&das);

            DBG2(dds2->print_xml(cerr, false, ""));

            AttrTable &at = dds2->get_attr_table();
            AttrTable::Attr_iter i = at.attr_begin();
            CPPUNIT_ASSERT(i != at.attr_end() && at.get_name(i) == "HDF_GLOBAL");
            CPPUNIT_ASSERT(i != at.attr_end() && at.get_name(++i) == "CoreMetadata");
        } catch (Error &e) {
            cout << "Error: " << e.get_error_message() << endl;
            CPPUNIT_FAIL("Error thrown!");
        }
    }

    void symbol_name_test() {
        try {
            // read a DDS.
            dds1->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.18");
            CPPUNIT_ASSERT(dds1->var("oddTemp"));

            dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19b");
            CPPUNIT_ASSERT(dds2->var("b#c"));
            CPPUNIT_ASSERT(dds2->var("b%23c"));
            CPPUNIT_ASSERT(dds2->var("huh.Image#data"));
            CPPUNIT_ASSERT(dds2->var("c d"));
            CPPUNIT_ASSERT(dds2->var("c%20d"));
        } catch (Error &e) {
            cerr << e.get_error_message() << endl;
            CPPUNIT_FAIL("Caught unexpected Error object");
        }
    }

    void print_xml_test() {
        try {
            dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19b");
            ostringstream oss;
            dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");
            DBG2(cerr << "Printed DDX: " << oss.str() << endl);

            string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19b.xml");
            DBG2(cerr << "The baseline: " << baseline << endl);

            CPPUNIT_ASSERT(baseline == oss.str());
        }
        catch (Error &e) {
            cerr << e.get_error_message() << endl;
            CPPUNIT_FAIL("Caught unexpected Error object");
        }
    }

    void print_xml_test2() {
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19c");
        DAS das;
        das.parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19c.das");

        dds2->transfer_attributes(&das);

        ostringstream oss;
        dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");

        DBG2(cerr << oss.str() << endl);

        string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19c.xml");
        DBG2(cerr << baseline << endl);
        CPPUNIT_ASSERT(baseline == oss.str());
    }

    void print_xml_test3() {
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19d");
        DAS das;
        das.parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19d.das");

        dds2->transfer_attributes(&das);

        ostringstream oss;
        dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");

        DBG2(cerr << oss.str() << endl);

        string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19d.xml");
        DBG2(cerr << baseline << endl);
        CPPUNIT_ASSERT(baseline == oss.str());
    }

    // This tests the HDF4 <var>_dim_n attribute. support for that was
    // moved to the handler itself.
    void print_xml_test3_1() {
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19d");
        DAS das;
        das.parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19d1.das");

        dds2->transfer_attributes(&das);

        ostringstream oss;
        dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");

        DBG2(cerr << oss.str() << endl);

        string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19d1.xml");
        DBG2(cerr << baseline << endl);
        CPPUNIT_ASSERT(baseline == oss.str());
    }

    void print_xml_test4() {
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19e");
        DAS das;
        das.parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19e.das");

        dds2->transfer_attributes(&das);

        DBG(AttrTable &at2 = dds2->var("c%20d")->get_attr_table()); DBG(at2.print(stderr));

        ostringstream oss;
        dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");

        DBG(cerr << oss.str() << endl);

        string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19e.xml");
        DBG2(cerr << baseline << endl);
        CPPUNIT_ASSERT(baseline == oss.str());
    }

    void print_xml_test5() {
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19f");
        DAS das;
        das.parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19f.das");

        try {
            dds2->transfer_attributes(&das);
        } catch (Error &e) {
            cerr << "Error: " << e.get_error_message() << endl;
            CPPUNIT_FAIL("Error exception");
        }

        DBG(AttrTable &at2 = dds2->var("huh")->get_attr_table()); DBG(at2.print(stderr));

        ostringstream oss;
        dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");

        DBG(cerr << oss.str() << endl);

        string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19f.xml");
        DBG2(cerr << baseline << endl);
        CPPUNIT_ASSERT(baseline == oss.str());
    }

    // Tests flat DAS into a DDS; no longer supported by libdap; specialize
    // handlers if they make these malformed DAS objects
    void print_xml_test5_1() {
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19f");
        DAS das;
        das.parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19f1.das");

        try {
            dds2->transfer_attributes(&das);
        } catch (Error &e) {
            cerr << "Error: " << e.get_error_message() << endl;
            CPPUNIT_FAIL("Error exception");
        }

        DBG(AttrTable &at2 = dds2->var("huh")->get_attr_table()); DBG(at2.print(stderr));

        ostringstream oss;
        dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");

        DBG(cerr << oss.str() << endl);

        string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19f1.xml");
        DBG2(cerr << baseline << endl);
        CPPUNIT_ASSERT(baseline == oss.str());
    }

    void print_xml_test6() {
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19b");
        DAS das;
        das.parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19b.das");

        dds2->transfer_attributes(&das);

        ostringstream oss;
        dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");

        DBG(cerr << oss.str() << endl);

        string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19b6.xml");
        DBG2(cerr << baseline << endl);
        CPPUNIT_ASSERT(baseline == oss.str());
    }

    // Tests flat DAS into a DDS; no longer supported by libdap; specialize
    // handlers if they make these malformed DAS objects
    void print_xml_test6_1() {
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19g");
        DAS das;
        das.parse((string) TEST_SRC_DIR + "/dds-testsuite/test.19g.das");

        try {
            dds2->transfer_attributes(&das);
        } catch (Error &e) {
            cerr << "Error: " << e.get_error_message() << endl;
            CPPUNIT_FAIL("Error exception");
        }

        DBG(AttrTable &at2 = dds2->var("huh")->get_attr_table()); DBG(at2.print(stderr));

        ostringstream oss;
        dds2->print_xml_writer(oss, false, "http://localhost/dods/test.xyz");

        DBG(cerr << oss.str() << endl);

        string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/test.19g.xml");
        DBG2(cerr << baseline << endl);
        CPPUNIT_ASSERT(baseline == oss.str());
    }

    void print_dmr_test()
    {
        try {
            dds_dap4->parse((string) TEST_SRC_DIR + "/dds-testsuite/fnoc1.nc.dds");
            DAS das;
            das.parse((string) TEST_SRC_DIR + "/dds-testsuite/fnoc1.nc.das");
            dds_dap4->transfer_attributes(&das);
#if 0
            string file = (string) TEST_SRC_DIR + "/dds-testsuite/fnoc1.nc.dmr.xml";
            ofstream d(file.c_str());
            dds_dap4->print_dmr(d, false);
            d.close();
#endif
            ostringstream oss;
            dds_dap4->print_dmr(oss, false);

            string baseline = readTestBaseline((string) TEST_SRC_DIR + "/dds-testsuite/fnoc1.nc.dmr.xml");

            DBG(cerr << "Baseline: -->" << baseline << "<--" << endl);
            DBG(cerr << "DMR: -->" << oss.str() << "<--" << endl);

            CPPUNIT_ASSERT(baseline == oss.str());
        }
        catch (Error &e) {
            CPPUNIT_FAIL(e.get_error_message());
        }
    }

    void get_response_size_test() {
        dds1->parse((string) TEST_SRC_DIR + "/dds-testsuite/3B42.980909.5.HDF.dds");
        CPPUNIT_ASSERT(dds1->get_request_size(false) == 230400);
        DBG(cerr << "3B42.980909.5.HDF response size: " << dds1->get_request_size(false) << endl);

        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/coads_climatology.nc.dds");
        CPPUNIT_ASSERT(dds2->get_request_size(false) == 3119424);
        DBG(cerr << "coads_climatology.nc response size: " << dds2->get_request_size(false) << endl);
    }

    void get_response_size_test_c() {
        ConstraintEvaluator eval;

        dds1->parse((string) TEST_SRC_DIR + "/dds-testsuite/3B42.980909.5.HDF.dds");
        eval.parse_constraint("percipitate", *dds1);
        DBG(cerr << "3B42.980909.5.HDF response size: " << dds1->get_request_size(true) << endl);
        CPPUNIT_ASSERT(dds1->get_request_size(true) == 115200);
        CPPUNIT_ASSERT(dds1->get_request_size(false) == 230400);

        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/coads_climatology.nc.dds");
        eval.parse_constraint("SST", *dds2);
        DBG(cerr << "coads_climatology.nc response size: " << dds2->get_request_size(true) << endl);
        CPPUNIT_ASSERT(dds2->get_request_size(true) == 779856);
        CPPUNIT_ASSERT(dds2->get_request_size(false) == 3119424);
    }

    void get_response_size_test_c2() {
        ConstraintEvaluator eval;
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/coads_climatology.nc.dds");
        eval.parse_constraint("SST[0:5][0:44][0:89]", *dds2);
        //cerr << "coads_climatology.nc response size: " << dds2->get_request_size(true) << endl;
        CPPUNIT_ASSERT(dds2->get_request_size(true) == 98328);
    }

    void get_response_size_test_c3() {
        ConstraintEvaluator eval;
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/coads_climatology.nc.dds");
        eval.parse_constraint("SST[0][0:44][0:89]", *dds2);
        DBG(cerr << "coads_climatology.nc response size: " << dds2->get_request_size(true) << endl);
        CPPUNIT_ASSERT(dds2->get_request_size(true) == 17288);
    }

    void get_response_size_test_seq() {
        ConstraintEvaluator eval;
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/S2000415.HDF.dds");
        eval.parse_constraint("NSCAT%20Rev%2020.NSCAT%20L2", *dds2);
        DBG(cerr << "S2000415.HDF response size: " << dds2->get_request_size(true) << endl);
        DBG(dds2->print_constrained(cerr));
        CPPUNIT_ASSERT(dds2->get_request_size(true) == 16
                || dds2->get_request_size(true) == 12);
        // sizeof(string) == 8 or 4 depending on the compiler version (?)
    }

    void get_response_size_test_seq_c() {
        ConstraintEvaluator eval;
        dds2->parse((string) TEST_SRC_DIR + "/dds-testsuite/S2000415.HDF.dds");
        eval.parse_constraint("NSCAT%20Rev%2020.NSCAT%20L2.Low_Wind_Speed_Flag", *dds2);
        DBG(cerr << "S2000415.HDF response size: " << dds2->get_request_size(true) << endl);
        CPPUNIT_ASSERT(dds2->get_request_size(true) == 4);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(DDSTest);

}

int main(int, char *[]) {
    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

    bool wasSuccessful = runner.run("", false);

    return wasSuccessful ? 0 : 1;
}

