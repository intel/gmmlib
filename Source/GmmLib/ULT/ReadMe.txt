========================================================================
    CONSOLE APPLICATION : GmmLibULT Project Overview
========================================================================
GMM ULT for the GMM Cache Policy.


GmmLibULT.vcxproj
    This is the main project file.

GmmLibULT.vcxproj.filters
    This is the filters file for VC++ project. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

GmmLibULT.cpp
    This is the main application source file.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named GmmLibULT.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:
    This ULT is divided into 2 parts.
	1. CompileTime ULT - Runs with every Gmmlib build and KMD build
		How to trigger Test cases through commandline: 
			i. Run all Compile Time TestCases	--> GmmULT.exe	CTest*.*
			ii.Run Specific TestCase			--> GmmULT.exe	CTestGen9CachePolicy.*

	2. RunTime ULT - Runnable on Target system. Have to run it manually - not qualified to run on Host/Dev systems 
		How to trigger Test cases through commandline:
			i. CachePolicy ULT					--> GmmULT.exe	RTestGen9CachePolicy.*
			ii. Vulkan Generic Resource			--> GmmULT.exe	RTestVulkanResource.*Generic*
			ii. Vulkan Sparse Resource			--> GmmULT.exe	RTestVulkanResource.*Sparse*
		To Run the Test on target
			i.  Download Driver along with Test Tools, which has ExCITE DLL or build ExCITE DLL on your dev system with the installed driver source
			ii. Install driver and copy DLL in either C:\Windows\System32 (for 64-bit app/DLL) or C:\Windows\SysWoW64 or place it in ULT executable Directory
			iii. Specify commandline and run GMMULT.exe


Test Case: 
	> Test Case is defined by FIXTURE class -> Test Case = FIXTURE Class
	> Ex. class CTestResource :  public testing::Test	--> CTestResource is FIXTURE class

Test:
	> Test resides in FIXTURE. FIXTURE class has multiple tests
	> Ex. TEST_F(CTestResource, test2DResource) --> test2DResource is a test of test case - CTestResource

SetUp() vs SetUpTestCase()
	> SetUp() -> gets called for all the tests in a test case. This is per-test setup/tear down
	> SetUpTestCase() -> When multiple tests in a test case share resource or needs same set up, then instead of repeating set up 
						 per test, SetUpTestCase gets called once per test case and all the tests inside a test case, use same set up.


To exclude tests from execution
	> --gtest_filter=POSTIVE_PATTERNS[-NEGATIVE_PATTERNS]
	> Ex. --gtest_filter=-ABC.*:BCD.*


What happens when test/test case is triggered
	TEST_F(CTestGen9Resource, Test2DTileYResource) --> first instance of test in CTestGen9Resource FIXTURE test case
		1. CTestGen9Resource::SetUpTestCase() --> this sets up platform --> This step is skipped if this is subsequence test instances in test case.
			i. Calls CommonULT::SetUpTestCase() --> GmmInitGlobalContext() and other initialization
		2. CTestResource::CTestResource()
		3. CTestGen9Resource::CTestGen9Resource()
		4. Test_F body --> test execution
		5. CTestGen9Resource::~CTestGen9Resource()
		6. CTestResource::~CTestResource( )
		7. CommonULT::~CommonULT() -->  Destroys Global Context
		8. void CTestGen9Resource::TearDownTestCase()  --> only if this is last instance of test in test case


CompileTime cmdline: $(TargetDir)$(TargetFileName) --gtest_filter=CTestResource.Test2DTileYsResource

To debug failures, add the following in the command line argument: --gtest_break_on_failure
This will cause an assert to be hit whenever a test fails. You can use the call stack to go back to the failing test and debug.
/////////////////////////////////////////////////////////////////////////////
