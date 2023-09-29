import os
import yaml

from pathlib import Path

from e2e_utils import exec_command, sprintln, Style

# ==================================================================================================

class E2eAnalyzer:
    """
    Analyzes E2E test reports.
    """

    # The keyword that can be passed to the first argument of the constructor instead of an
    # actually existing directory path. It will be replaced by the path to the directory containing
    # all report files produced during the *last* execution of the 'run' subcommand.
    LATEST_TEST_RESULT_DIR: str = "[latest-test-result-dir]"

    def __init__(self, dir_path: str, report_file_name_mask: str, vscode: bool = False) -> None:
        """
        Initializes E2eAnalyzer.

        Args:
            dir_path:
                The path to the directory to recursively scan for report files. If set to
                {LATEST_TEST_RESULT_DIR}, it will be replaced by the path to the directory
                containing all report files produced during the *last* execution of the 'run'
                subcommand.
            file_mask:
                The file mask to be used to match the report files, e.g.: "*.report".
            vscode:
                Whether to open the expected and actual output of each failed test case in VS code
                side-by-side, with the differences highlighted in color.
        """
        if dir_path == E2eAnalyzer.LATEST_TEST_RESULT_DIR:
            self.dir_path = self.detect_latest_test_result_dir()
        else:
            self.dir_path = Path(dir_path)
        self.report_file_name_mask = report_file_name_mask
        self.vscode = vscode

    def run(self) -> None:
        """
        Validates the input parameters, scans the directory for reports of failed test cases and
        analyzes the test cases.
        """

        # Validate the path to the directory to scan.
        if not self.dir_path:
            sprintln("No path to a directory to scan given.", Style.ERROR)
            exit(1)
        if not self.dir_path.is_dir():
            sprintln(f"The path '{self.dir_path}' is not a directory.", Style.ERROR)
            exit(1)

        # Validate the file name mask.
        if not self.report_file_name_mask:
            sprintln("No file name mask to match the report files given.", Style.ERROR)
            exit(1)

        # Print the preamble.
        sprintln("Analyzing E2E test reports", Style.TITLE)
        sprintln("=" * 100, Style.SEPARATOR_LINE)
        sprintln(f"• directory path:   {self.dir_path.resolve()}", Style.PREAMBLE)
        sprintln(f"• report file mask: {self.report_file_name_mask}", Style.PREAMBLE)
        sprintln(f"• VS code mode:     {self.vscode}", Style.PREAMBLE)
        sprintln("=" * 100, Style.SEPARATOR_LINE)

        # Scan the directory for reports of failed test cases.
        sprintln("Detecting reports of failed test cases ... ", style=Style.TASK_HEADING)
        reports = self.detect_reports_of_failed_test_cases()
        sprintln(f"• Number of found reports: {len(reports)}")

        # Analyze the reports.
        self.analyze_reports(reports)

    def detect_reports_of_failed_test_cases(self) -> dict[Path, dict]:
        """
        Scans {self.dir_path} recursively for report files, that is: for files matching
        {self.report_file_name_mask}). Parses each report file for the contained key/value pairs
        (it is expected that each report file is in YAML format) and stores them in form of a
        dictionary (a so called report dictionary). Adds a report dictionary to the result
        dictionary if its value "ok" is set to False.

        Returns:
            A dictionary mapping report file paths to report dictionaries. It only contains those
            report dictionaries whose value "ok" is set to False.
        """
        reports = {}

        # Scan the directory recursively for files matching the file name mask.
        entry: Path
        for entry in self.dir_path.rglob(self.report_file_name_mask):
            # Ignore the entry if it is a directory.
            if entry.is_dir():
                continue

            entry_rel = entry.relative_to(self.dir_path)

            # Load the file. Ignore it if it can't be read or it is not in YAML format.
            try:
                with open(entry, "r") as f:
                    report = yaml.safe_load(f)
            except Exception as e:
                e_first_line = str(e).split("\n")[0]
                sprintln(f"• ignoring '{entry_rel}' ({e_first_line})", Style.WARN)
                continue

            # Ignore the report if it is empty.
            if not report:
                sprintln(f"• ignoring '{entry_rel}' (is empty)", Style.WARN)
                continue

            if type(report) != dict:
                sprintln(f"• ignoring '{entry_rel}' (not in YAML format)", Style.WARN)
                continue

            ok = report.get("ok")

            # Ignore the report if it does not contain an 'ok' entry.
            if ok is None:
                sprintln(f"• ignoring '{entry_rel}' (doesn't contain 'ok' entry)", Style.WARN)
                continue

            # Ignore the report if the test case succeeded.
            if ok:
                continue

            reports[entry] = report

        return reports

    def analyze_reports(self, reports: dict[Path, dict]) -> None:
        """
        Analyzes the given reports.
        """

        if not reports:
            return

        sum_num_inserts = 0
        max_num_inserts = float("-inf")
        min_num_inserts = float("inf")

        sum_num_deletes = 0
        max_num_deletes = float("-inf")
        min_num_deletes = float("inf")

        sprintln("=" * 100, Style.SEPARATOR_LINE)

        for i, report_path in enumerate(reports):
            report = reports[report_path]
            prefix = f"[{i+1}/{len(reports)}] "
            report_rel_file_path = report_path.relative_to(self.dir_path)
            num_inserts = report.get("diff", {}).get("num_inserts", 0)
            num_deletes = report.get("diff", {}).get("num_deletes", 0)

            sprintln(f"{prefix}Analyzing report '{report_rel_file_path}'...", Style.TASK_HEADING)
            sprintln(f"• #inserts: {num_inserts}; #deletes: {num_deletes}")

            sum_num_inserts += num_inserts
            min_num_inserts = min(min_num_inserts, num_inserts)
            max_num_inserts = max(max_num_inserts, num_inserts)

            sum_num_deletes += num_deletes
            min_num_deletes = min(min_num_deletes, num_deletes)
            max_num_deletes = max(max_num_deletes, num_deletes)

            if self.vscode:
                try:
                    self.analyze_in_vscode(report)
                except Exception as e:
                    sprintln(f"• analyzing in VS code failed: '{str(e)}'", Style.WARN)

        # Compute the average number of inserts and deletes.
        avg_num_inserts: float = sum_num_inserts / len(reports) if len(reports) > 0 else 0
        avg_num_deletes: float = sum_num_deletes / len(reports) if len(reports) > 0 else 0

        sprintln("=" * 100, Style.SEPARATOR_LINE)
        sprintln("    | inserts | deletes |", Style.SUMMARY)
        sprintln("----+---------+---------+", Style.SUMMARY)
        sprintln(f"sum | {sum_num_inserts: 7} | {sum_num_deletes: 7} |", Style.SUMMARY)
        sprintln(f"min | {min_num_inserts: 7} | {min_num_deletes: 7} |", Style.SUMMARY)
        sprintln(f"max | {max_num_inserts: 7} | {max_num_deletes: 7} |", Style.SUMMARY)
        sprintln(f"avg | {avg_num_inserts: 7.1f} | {avg_num_deletes: 7.1f} |", Style.SUMMARY)
        sprintln("")
        sprintln(f"total number of analyzed reports: {len(reports)}", Style.SUMMARY)
        sprintln("")

    def analyze_in_vscode(self, report: dict) -> None:
        """
        Opens the expected output file (stored in the given dictionary under the key
        "paths.expected_output_file") and the actual output file (stored under
        "paths.actual_output_file") in Visual Studio code side-by-side, with the changes
        between the files highlighted in color.

        Args:
            report
                The report containing the paths to the expected output file and actual output file.
        """

        if not report:
            raise ValueError("no report given.")

        if type(report) != dict:
            raise ValueError("report is not a dictionary.")

        expected_output_file_path: str = report.get("paths", {}).get("expected_output_file")
        if not expected_output_file_path:
            raise ValueError("no expected output file path given")

        actual_output_file_path: str = report.get("paths", {}).get("actual_output_file")
        if not actual_output_file_path:
            raise ValueError("no actual output file path given")

        cmd = f"code --diff {expected_output_file_path} {actual_output_file_path}"
        status, _, stderr = exec_command(cmd)

        if status > 0 or stderr:
            raise ValueError(f"{stderr} ({status})")

    def detect_latest_test_result_dir(self) -> Path:
        """
        Returns the path to the directory containing all report files produced during
        the *last* execution of the 'run' subcommand.
        """

        # TODO: Do not hardcode the path to the results directory.
        # It can change at any time, by modifying the paths in the config file.
        base_dir: Path = Path(__file__).parent / "results"

        if not base_dir.exists():
            sprintln("Error on detecting latest test result dir: "
                     f"'{base_dir}' does not exist", Style.ERROR)
            exit(1)

        sub_dirs: list[Path] = list(base_dir.glob("*/"))

        if not sub_dirs:
            sprintln("Error on detecting latest test result dir: "
                     f"'{base_dir}' does not contain subdirectories", Style.ERROR)
            exit(1)

        return max(sub_dirs, key=os.path.getmtime)