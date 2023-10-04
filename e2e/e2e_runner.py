import yaml

from datetime import datetime
from pathlib import Path

from e2e_utils import exec_command, format, read_config_file, sprint, sprintln, wdiff
from e2e_utils import E2eConfig, E2eTest, E2eTestCase, Style

# ==================================================================================================

# The format string to be used for formatting a datetime occurring in a logging message.
DATETIME_LOG_FORMAT: str = "%Y-%m-%d %H:%M:%S.%f"

# The format string to be used for formatting a datetime occurring in a file path.
DATETIME_FILENAME_FORMAT: str = "%Y-%m-%d_%H-%M-%S"

# ==================================================================================================


class E2eRunner:
    """
    Runs the E2E tests of pdftotext++.
    """

    def __init__(self, ppp_path: Path, config_file_path: Path) -> None:
        """
        Initializes E2eRunner.

        Args:
            ppp_path:
                The (absolute) path to the pdftotext++ executable.
            config_file_path:
                The (absolute) path to the config file in which the E2E tests are specified.
        """
        self.ppp_path = ppp_path
        self.config_file_path = config_file_path

        # The last modified date of the pdftotext++ executable (for debugging purposes).
        self.ppp_mod_date = None
        # The version of the pdftotext++ executable (for debugging purposes).
        self.ppp_version = None
        # The datetime of starting the run() method (for debugging purposes).
        self.e2e_run_date = None

    def run(self) -> None:
        """
        Validates the input parameters, reads the config file, and runs the E2E tests specified in
        the config file.
        """

        # Validate the path to the pdtotext++ executable.
        if not self.ppp_path:
            sprintln("No path to the pdftotext++ executable given.", Style.ERROR)
            exit(1)
        if not self.ppp_path.is_file():
            sprintln(f"The path '{self.ppp_path}' is not a file.", Style.ERROR)
            exit(1)

        # Validate the path to the config file.
        if not self.config_file_path:
            sprintln("No path to a config file given.", Style.ERROR)
            exit(1)
        if not self.config_file_path.is_file():
            sprintln(f"The path '{self.config_file_path}' is not a file.", Style.ERROR)
            exit(1)

        # Get the last modified date of the pdftotext++ executable.
        ppp_mod_date_cmd: str = f"stat -c '%y' '{self.ppp_path}'"
        status, self.ppp_mod_date, stderr = exec_command(ppp_mod_date_cmd, strip_output=True)
        if status > 0 or not self.ppp_mod_date:
            sprintln("Could not get the last mod date of the pdftotext++ executable.", Style.ERROR)
            sprintln(f"status: {status}")
            sprintln(f"stdout: {self.ppp_mod_date}")
            sprintln(f"stderr: {stderr}")
            exit(1)

        # Get the version of the pdftotext++ executable.
        ppp_version_cmd: str = f"{self.ppp_path} --version"
        status, self.ppp_version, stderr = exec_command(ppp_version_cmd, strip_output=True)
        if status > 0 or not self.ppp_version:
            sprintln("Could not get the version from the pdftotext++ executable.", Style.ERROR)
            sprintln(f"status: {status}")
            sprintln(f"stdout: {self.ppp_version}")
            sprintln(f"stderr: {stderr}")
            exit(1)

        # Get the current date.
        self.e2e_run_date = datetime.now()
        e2e_run_date_str = self.e2e_run_date.strftime(DATETIME_LOG_FORMAT)

        # Print the preamble.
        sprintln("Running E2E tests", Style.TITLE)
        sprintln("=" * 100, Style.SEPARATOR_LINE)
        sprintln(f"• start date:          {e2e_run_date_str}", Style.PREAMBLE)
        sprintln(f"• config file path:    {self.config_file_path.resolve()}", Style.PREAMBLE)
        sprintln(f"• pdftotext++ path:    {self.ppp_path.resolve()}", Style.PREAMBLE)
        sprintln(f"• pdftotext++ date:    {self.ppp_mod_date}", Style.PREAMBLE)
        sprintln(f"• pdftotext++ version: {self.ppp_version}", Style.PREAMBLE)
        sprintln("=" * 100, Style.SEPARATOR_LINE)

        # Read the config file.
        try:
            config = read_config_file(self.config_file_path)
        except Exception as e:
            sprintln("Could not read the config file.", Style.ERROR)
            sprintln(f"error: {str(e)}")
            exit(1)

        # Run the E2E tests specified in config.tests.
        self.run_tests(config)

    def run_tests(self, config: E2eConfig) -> None:
        """
        Runs the E2E tests specified in the given config.

        Args:
            config:
                The config. The E2E tests to be run are expected to be stored in config.tests.
        """

        # Abort if there is no config.
        if not config:
            sprintln("No config given.", Style.ERROR)
            exit(1)

        # Abort if there are no tests.
        if not config.tests:
            sprintln("No tests given.", Style.ERROR)
            exit(1)

        num_cases = 0
        num_cases_skipped = 0
        num_cases_exceptions = 0
        num_cases_failures = 0
        num_cases_passed = 0

        for i, test in enumerate(config.tests):
            name: str = test.name
            pre = f"[{i+1}/{len(config.tests)}] "

            # Detect the test cases of the test.
            try:
                cases: list[E2eTestCase] = self.detect_test_cases(config, test)
            except Exception as e:
                sprintln(f"{pre}Skipped test '{name}' (error on detecting test cases)", Style.WARN)
                sprintln(f"error: {str(e)}")
                continue

            if not cases:
                sprintln(f"{pre}Skipped test '{name}' (no test cases detected)", Style.WARN)
                continue

            sprintln(f"{pre}Running test '{name}'...", Style.TASK_HEADING)
            num_cases += len(cases)

            # Run the test cases of the test.
            for j, case in enumerate(cases):
                head = f" • case {j + 1}: {case.cmd_short} "
                sprint(head, flush=True)

                # Evaluate the test case.
                try:
                    num_ins, num_dels = self.evaluate_test_case(case)
                except Exception as e:
                    sprintln(f"\r{head}[EXCEPTION] {str(e)}", Style.EXCEPTION)
                    num_cases_exceptions += 1
                    continue

                if num_ins > 0 or num_dels > 0:
                    sprintln(f"\r{head}[FAILED] {num_ins} inserts, {num_dels} deletes", Style.ERROR)
                    num_cases_failures += 1
                    continue

                sprintln(f"\r{head}[OK]", Style.SUCCESS)
                num_cases_passed += 1

            if i < len(config.tests) - 1:
                sprintln("-" * 100, Style.SEPARATOR_LINE)

        # Print a summary of the executed test cases.
        sprintln("=" * 100, Style.SEPARATOR_LINE)
        sprintln(f"number of test cases: {num_cases} "
                 f"(skipped: {num_cases_skipped};"
                 f" exceptions: {num_cases_exceptions};"
                 f" passed: {num_cases_passed};"
                 f" failures: {num_cases_failures})", Style.SUMMARY)
        sprint("")

    # ==============================================================================================

    def detect_test_cases(self, config: E2eConfig, test: E2eTest) -> list[E2eTestCase]:
        """
        Scans the directory {config.paths.pdfs_dir} for PDF files. For each detected PDF file,
        checks if an expected output file, defined by {test.expected_output_file_path_pattern},
        exists. For each detected (PDF file, expected output file) pair, creates an E2eTestCase
        object and appends it to the result list.

        Args:
            config:
                The E2E test config.
            test:
                The test for which to detect the test cases.

        Returns:
            The list of detected (PDF file, expected output file) pairs, each represented by an
            E2eTestCase object.
        """

        if not config:
            raise ValueError("No config given.")

        if not config.pdfs_dir_path:
            raise ValueError("The config does not contain a path to a PDF directory.")

        if not test:
            raise ValueError("No test given.")

        if not test.slug:
            raise ValueError("No test slug given.")

        if not test.ppp_args_pattern:
            raise ValueError("No arguments pattern for the pdftotext++ command given.")

        if not test.expected_output_file_path_pattern:
            raise ValueError("No expected output file path pattern given.")

        if not test.actual_output_file_path_pattern:
            raise ValueError("No actual output file path pattern given.")

        if not test.diff_file_path_pattern:
            raise ValueError("No diff file path pattern given.")

        if not test.report_file_path_pattern:
            raise ValueError("No report file path pattern given.")

        e2e_run_date_str: str = self.e2e_run_date.strftime(DATETIME_FILENAME_FORMAT)

        # Detect (PDF file, expected output file) pairs.
        cases: list[E2eTestCase] = []
        for pdf in config.pdfs_dir_path.glob("*.[pP][dD][fF]"):
            # Define the path to the expected output file.
            expected_output_file_path = Path(format(
                test.expected_output_file_path_pattern,
                test_slug=test.slug,
                pdf_stem=pdf.stem
            ))

            # Skip the PDF if no expected output file exists.
            if not expected_output_file_path.exists():
                continue

            case = E2eTestCase()
            case.pdf_file_path = pdf
            case.expected_output_file_path = expected_output_file_path
            case.cmd = f"{self.ppp_path} {format(test.ppp_args_pattern, pdf=pdf)}"
            case.cmd_short = f"{self.ppp_path.name} {format(test.ppp_args_pattern, pdf=pdf.name)}"

            # Define the path to the file into which to write the actual output of {case.cmd}.
            case.actual_output_file_path = Path(format(
                test.actual_output_file_path_pattern,
                e2e_run_date=e2e_run_date_str,
                test_slug=test.slug,
                pdf_stem=pdf.stem
            ))

            # Define the path to the file into which to write the output produced by diff.
            case.diff_file_path = Path(format(
                test.diff_file_path_pattern,
                e2e_run_date=e2e_run_date_str,
                test_slug=test.slug,
                pdf_stem=pdf.stem
            ))

            # Define the path to the file into which to write the report.
            case.report_file_path = Path(format(
                test.report_file_path_pattern,
                e2e_run_date=e2e_run_date_str,
                test_slug=test.slug,
                pdf_stem=pdf.stem
            ))

            cases.append(case)

        return cases

    def evaluate_test_case(self, case: E2eTestCase) -> tuple[int, int]:
        """
        Executes {case.cmd} and writes the produced output to {case.actual_output_file_path}.
        Compares the two files {case.actual_output_file_path} and {case.expected_output_file_path}
        using the tool 'diff' and writes the diff result to {case.diff_file_path}. Creates a report
        (containing metadata about the test case) and writes it to {case.report_file_path}.

        Args:
            case:
                The test case to evaluate.

        Returns:
            A pair (num_inserts, num_deletes), where {num_inserts} is the number of insertions and
            {num_deletes} is the number of deletions necessary to transform the actual output
            (produced by {case.cmd}) to the expected output.
        """
        # Run {case.cmd} to get the actual output.
        status, actual_output, stderr = exec_command(case.cmd)
        if status > 0 or stderr:
            raise ValueError(f"Error on running \"{case.cmd_short}\": {stderr} ({status})")

        # Write the actual output to file.
        case.actual_output_file_path.parent.mkdir(parents=True, exist_ok=True)
        case.actual_output_file_path.write_text(actual_output)

        # Compute the diff result and write it to file.
        case.diff_result, case.num_inserts, case.num_deletes = wdiff(
            case.actual_output_file_path,
            case.expected_output_file_path
        )
        case.diff_file_path.parent.mkdir(parents=True, exist_ok=True)
        case.diff_file_path.write_text(case.diff_result)

        # Create the report and write it to file.
        report = self.create_test_case_report(case)
        case.report_file_path.parent.mkdir(parents=True, exist_ok=True)
        case.report_file_path.write_text(yaml.dump(report))

        return case.num_inserts, case.num_deletes

    def create_test_case_report(self, case: E2eTestCase) -> dict:
        """
        Creates a dictionary containing metadata about the given test case (for example: whether or
        not the test case succeeded, or the pdftotext++ command executed by the test case).

        Args:
            case:
                The test case for which to create the dictionary.

        Returns:
            A dictionary containing metadata about the test case.
        """

        return {
            "cmd": {
                "full": case.cmd,
                "short": case.cmd_short
            },
            "ok": case.num_inserts == 0 and case.num_deletes == 0,
            "diff": {
                "num_inserts": case.num_inserts,
                "num_deletes": case.num_deletes,
            },
            "date": self.e2e_run_date.strftime(DATETIME_LOG_FORMAT),
            "paths": {
                "pdf": str(case.pdf_file_path.resolve()),
                "actual_output_file": str(case.actual_output_file_path.resolve()),
                "expected_output_file": str(case.expected_output_file_path.resolve()),
                "diff_file": str(case.diff_file_path.resolve()),
                "config_file": str(self.config_file_path.resolve())
            },
            "ppp": {
                "executable": str(self.ppp_path.resolve()),
                "version": self.ppp_version,
                "modification_date": self.ppp_mod_date,
            }
        }
