#
#      \V (O |R |P /A |L |I |N |E
# (C) Bruno Levy, INRIA - ALICE, 2012,2013
#
#   Confidential - proprietary software
#
# This file contains the RobotFramework test library for executing test
# programs in the framework of the Vorpaline project.
#

import os
import shutil
import sys
import subprocess
from robot.api import logger
from robot.libraries.BuiltIn import BuiltIn

__version__ = '0.1'

# These variables are used for testing the VorpatestLibrary
# as a standalone executable
_MOCKUP_TEST_VARIABLES = {
    '${EXECDIR}': '.',
    '${SUITE SOURCE}': '.',
    '${TEST NAME}': 'exec.dir',
    '${TEST STATUS}': 'PASS'
}

def _get_test_variables():
    """
    Returns the dictionary of test variables.

    If the library is invoked as a standalone executable,
    return a mockup dictionary, otherwise return the RobotFramework
    test variable dictionary.
    """
    if __name__ == "__main__":
        return _MOCKUP_TEST_VARIABLES
    else:
        return BuiltIn().get_variables()



##############################################################################
# Vorpaline execution wrappers (valgrind, callgrind, ...)

class _ExecutionWrapper:
    """
    Execution wrapper base class.
    Actual execution wrappers must reimplement function wrap_command()
    """

    # Index used to generate output file names and guarantee their uniqueness
    # during the whole suite execution.
    _file_index = 1


    def wrap_command(self, prog_name, command, preserve_files):
        """
        Returns a new command that executes the specified command under
        control of the wrapper.

        Arguments:
        prog_name -- name of the executable
        command -- the command to execute
        preserve_files -- must be filled with the list of files (log-files,
        output-files) to preserve at the end of the test execution
        """
        raise NotImplementedError("Pure virtual method wrap_command() called")


    def output_filename(self, prefix, suffix):
        """
        Generates a output filename from a prefix and a suffix.
        """
        filename = "%s%s%s" % (prefix, _ExecutionWrapper._file_index, suffix)
        _ExecutionWrapper._file_index += 1
        return filename


class _Valgrind(_ExecutionWrapper):
    """ Valgrind execution wrapper """

    _command = [
        'valgrind',
        '--verbose',
        '--xml=yes',
        '--leak-check=full',
        '--show-reachable=yes',
        '--error-limit=no',
    ]

    _outfile_ext = '.memcheck'
    _option_outfile = '--xml-file=%s'
    _suppressions_ext = '.supp'
    _option_suppressions = '--suppressions=%s'

    def wrap_command(self, prog_name, command, preserve_files):
        out_file = self.output_filename(prog_name, _Valgrind._outfile_ext)
        preserve_files.append(out_file)
        options = [_Valgrind._option_outfile % out_file]

        # Check suppression files:

        test_variables = _get_test_variables()

        supression_files = [
            # Suppression file given by option --with-valgrind-suppressions=...
            os.getenv('VORPALINE_WITH_VALGRIND_SUPPRESSIONS'),

            # Suppression file present in the test suite directory
            os.path.join(test_variables['${SUITE SOURCE}'], 'valgrind.supp'),

            # Suppression file present in the execution directory
            os.path.join(test_variables['${EXECDIR}'], 'valgrind.supp'),
        ]

        for file in supression_files:
            #sys.stderr.write("** Checking suppression file %s\n" % file)
            if (file != None) and os.path.isfile(file):
                options += [_Valgrind._option_suppressions % file]
                break

        return _Valgrind._command + options + command


class _Callgrind(_ExecutionWrapper):
    """ Callgrind execution wrapper """

    _command = [
        'callgrind',
    ]

    _outfile_ext = '.callgrind'
    _option_outfile = '--callgrind-out-file=%s'

    def wrap_command(self, prog_name, command, preserve_files):
        out_file = self.output_filename(prog_name, _Callgrind._outfile_ext)
        preserve_files.append(out_file)
        return _Callgrind._command + \
            [_Callgrind._option_outfile % out_file] + \
            command


##############################################################################
# RobotFramework test library for Vorpaline

class VorpatestLibrary:
    """
    RobotFramework test library for Vorpaline.

    It defines 3 main functions that can be invoked from RobotFramework test
    cases. Each function relies on environment variable VORPALINE_BIN_DIR to
    locate programs to execute:
    run_vorpaline -- executes vorpaline
    run_vorpastat -- executes vorpastat
    run_command -- executes any program
    """

    # Static variables used by all testcase executions
    _initialized = False
    _bin_dir = None
    _test_index = None
    _exec_wrapper = None

    def __init__(self):
        """
        Initializes the test library instance.
        Note that a new instance of the the library is created for EACH
        testcase execution. Testsuite globals must be kept in VorpatestLibrary
        static class variables.
        """

        if VorpatestLibrary._initialized:
            return

        sys.stderr.write("** Initializing VorpatestLibrary\n")

        VorpatestLibrary._initialized = True

        # Initialize the path to the vorpaline executables
        VorpatestLibrary._bin_dir = os.getenv('VORPALINE_BIN_DIR')
        if VorpatestLibrary._bin_dir is None:
            raise RuntimeError("Environment variable VORPALINE_BIN_DIR is not set")

        # Check for execution with valgrind
        if os.getenv('VORPALINE_WITH_VALGRIND') != None:
            VorpatestLibrary._exec_wrapper = _Valgrind()

        # Check for execution with callgrind
        elif os.getenv('VORPALINE_WITH_CALLGRIND') != None:
            VorpatestLibrary._exec_wrapper = _Callgrind()

        # Initialize the test index
        VorpatestLibrary._test_index = 0


    ######################################################################
    # Public functions:

    def prepare_test(self):
        """
        Prepares Vorpaline test execution.

        This function creates the test execution directory where vorpaline
        output file will be stored.
        """

        VorpatestLibrary._test_index += 1
        self._status = -1
        self._preserve_files = []

        test_variables = _get_test_variables()

        test_name = test_variables['${TEST NAME}']
        logger.info("Setup test %s" % test_name)

        test_name = test_name.replace(' ', '_').replace('.', '_')
        test_name = "%03d_%s" % (VorpatestLibrary._test_index, test_name)

        self._execdir = os.path.join(test_variables['${EXECDIR}'], 'run', test_name)
        self._log("Execution directory: %s" % self._execdir)

        shutil.rmtree(self._execdir, True)
        os.makedirs(self._execdir)
        os.chdir(self._execdir)


    def run_vorpaline(self, input_file, *options):
        """
        Run Vorpaline on the specified input file.
        The output file "out.meshb" is implicitely generated in the test
        execution directory created by prepare_test().

        Arguments:
        input_file -- path to the vorpaline input file
        options -- vorpaline options
        """

        self._input_file = input_file
        args = list(options) + [self._input_file, 'out.meshb']
        self._run_command("vorpaline", args)


    def run_vorpastat(self, *options):
        """
        Run Vorpastat to compare the vorpaline input file and the output file
        "out.meshb" generated by run_vorpaline.
        TODO: the output of vorpastats must be later inspected to check
        various metrics against the expected values.

        Arguments:
        options -- vorpastat options
        """

        args = list(options) + [self._input_file, 'out.meshb']
        self._run_command("vorpastat", args)


    def run_command(self, prog_name, *args):
        """
        Run a command and capture the standard and error output

        Arguments:
        prog_name -- name of the executable
        args -- command arguments given as a list
        """
        self._run_command(prog_name, list(args))


    def cleanup_test(self):
        """
        Cleanup after successful test execution.

        This removes the execution directory created by prepare_test(), only
        if the test was successful. Execution directories of failed tests are
        preserved for later inspection.
        """
        test_variables = _get_test_variables()
        test_status = test_variables['${TEST STATUS}']
        if test_status == 'FAIL':
            # Test failed: keep directory for later inspection
            return

        # Test passed: remove the execution directory but preserve all
        # important log files, if any (valgrind, gcov, ...)

        if len(self._preserve_files) == 0:
            shutil.rmtree(self._execdir, True)
            return

        # Move all the files to preserve to a temporary directory

        backup_dir = self._execdir + '.preserve'
        os.makedirs(backup_dir)
        for file in self._preserve_files:
            shutil.move(file, backup_dir)

        # Delete the execution directory and rename the temporary directory

        shutil.rmtree(self._execdir, True)
        os.rename(backup_dir, self._execdir)


    ######################################################################
    # Private functions:

    def _run_command(self, prog_name, args):
        """
        Run a command and capture the standard and error output

        This function executes the given command under control of the
        subprocess library. If the execution is driven by valgrind (Unix
        only), the command is wrapped in a valgrind command and then executed.

        Arguments:
        prog_name -- name of the executable
        args -- command arguments given as a list
        """

        # Build the command with the fullpath to the executable

        command = [os.path.join(VorpatestLibrary._bin_dir, prog_name)] + args

        # Debug: uncomment the following to debug the library for valgrind
        # executions.
        # command = ['/home/vorpatest/bin/dummy_vorpaline.sh'] + command

        # Check if execution is controlled by a wrapper

        if VorpatestLibrary._exec_wrapper != None:
            command = VorpatestLibrary._exec_wrapper.wrap_command(
                prog_name, command, self._preserve_files
            )

        self._log("Run command: %s" % command)
        self._status = -1

        try:
            output = subprocess.check_output(command, stderr=subprocess.STDOUT)
            self._log("Command passed")
            self._log("Output: %s" % output)
        except subprocess.CalledProcessError as e:
            self._log("Command failed!")
            self._log("Return code: %s" % e.returncode)
            self._log("Exception: %s" % e)
            self._log("Output: %s" % e.output)
            raise
        except:
            (exc_type, exc_value) = sys.exc_info()[:2]
            self._log("Command failed!")
            self._log("Exception type: %s" % exc_type)
            self._log("Exception value: %s" % exc_value)
            raise

        self._status = 0


    def _log(self, args):
        print "*INFO*",args,"\n"


######################################################################
# Define a main program for testing VorpatestLibrary

def main():
    """ For testing the VorpatestLibrary """
    testlib = VorpatestLibrary()
    testlib.prepare_test()
    testlib.run_vorpaline(*sys.argv[1:])
    testlib.run_vorpastat()
    testlib.cleanup_test()

if __name__ == "__main__":
    main()

