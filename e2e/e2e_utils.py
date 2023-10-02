import subprocess
import sys
import yaml

from os.path import isabs, join
from pathlib import Path

# ==================================================================================================


class E2eTestCase:
    """
    The specification of a single test case.
    """

    # The pdftotext++ command to be executed by this test case, with fully qualified file paths of
    # the pdftotext++ executable and the PDF file to be processed.
    cmd: str = None

    # The pdftotext++ command to be executed by this test case, with the pdftotext++ executable and
    # the PDF file specified only by their file names (and not their fully qualified file paths
    # like in {cmd}). This is useful for debugging purposes.
    cmd_short: str = None

    # The (absolute) path to the PDF file to be processed by this test case.
    pdf_file_path: Path = None

    # The (absolute) path to the file containing the expected output for this test case.
    expected_output_file_path: Path = None

    # The (absolute) path to the file into which to write the output produced by {cmd}.
    actual_output_file_path: Path = None

    # The (absolute) path to the file into which to write {diff_result}.
    diff_file_path: Path = None

    # The (absolute) path to the file into which to write the report after the test case completed.
    report_file_path: Path = None

    # The diff result between the expected and the actual output.
    diff_result: str = None

    # The number of insertions necessary to transform the actual output into the expected output.
    num_inserts: int = None

    # The number of deletions necessary to transform the actual output into the expected output.
    num_deletes: int = None


class E2eTest:
    """
    The specification of a test.
    """

    # The name of the test.
    name: str = None

    # The file slug of the test (required to, for example, identify the subdirectory containing the
    # expected output files for this test in the ground truth).
    slug: str = None

    # The pattern of the arguments to be passed to the pdftotext++ executable by a test case of
    # this test. It may contain the placeholder '{pdf}' which will be replaced by the absolute path
    # to the PDF file associated with the test case.
    ppp_args_pattern: str = None

    # The file path pattern of the files containing the expected outputs for the test cases.
    # It may contain the placeholder '{test_slug}' which will be replaced by the file slug of this
    # test and '{pdf_stem}' which will be replaced by the stem (= the file name without the file
    # extension) of the PDF file associated with the test case.
    expected_output_file_path_pattern: str = None

    # The file path pattern of the files into which to write the actual outputs of the test cases.
    # It may contain the placeholder '{e2e_run_date}', which will be replaced by the start date of
    # 'e2e.py run', the placeholder '{test_slug}' which will be replaced by the file slug of this
    # test and '{pdf_stem}' which will be replaced by the stem (= the file name without the file
    # extension) of the PDF file associated with the test case.
    actual_output_file_path_pattern: str = None

    # The file path pattern of the files into which to write the diff result between the actual
    # outputs and the expected outputs. It may contain the placeholder '{e2e_run_date}', which will
    # be replaced by the start date of 'e2e.py run', the placeholder '{test_slug}' which will be
    # replaced by the file slug of this test and '{pdf_stem}' which will be replaced by the stem
    # (= the file name without the file extension) of the PDF file associated with the test case.
    diff_file_path_pattern: str = None

    # The file path pattern of the files into which to write the reports of the test cases. It may
    # contain the placeholder '{e2e_run_date}', which will be replaced by the start date of
    # 'e2e.py run', the placeholder '{test_slug}' which will be replaced by the file slug of this
    # test and '{pdf_stem}' which will be replaced by the stem (= the file name without the file
    # extension) of the PDF file associated with the test case.
    report_file_path_pattern: Path = None


class E2eConfig:
    """
    The configuration to use for E2E testing.
    """

    # The path to the directory containing the PDF files to be processed by the E2E tests.
    pdfs_dir_path: Path = None

    # The specification of the E2E tests to be executed.
    tests: list[E2eTest] = None

# ==================================================================================================
# Config.


def read_config_file(config_file_path: Path) -> E2eConfig:
    """
    Reads the given config file. The expected format of the config file is as shown in config.yml.

    Args:
        config_file_path:
            The absolute path to the config file to read.

    Returns:
        The config in form of an E2eConfig object.
    """

    def to_abs(path: str) -> str:
        """
        If the given path is relative, interprets it as relative to the config file and makes it
        absolute by prepending the path to the parent directory of the config file.

        Args:
            path
                The path to process.

        Returns:
            The path, translated to an absolute path as described above.
        """
        if path is None:
            return

        if not isabs(path):
            return join(config_file_path.parent, path)

        return path

    # Load the config file.
    yml = yaml.safe_load(config_file_path.read_text())

    config = E2eConfig()
    config.pdfs_dir_path = Path(to_abs(yml["paths"]["pdfs_dir"]))
    config.tests = []

    # Read the tests.
    for t in yml["tests"]:
        test = E2eTest()
        test.name = t["name"]
        test.slug = t["slug"]
        test.ppp_args_pattern = t["ppp_args"]
        test.expected_output_file_path_pattern = to_abs(yml["paths"]["expected_output_file"])
        test.actual_output_file_path_pattern = to_abs(yml["paths"]["actual_output_file"])
        test.diff_file_path_pattern = to_abs(yml["paths"]["diff_file"])
        test.report_file_path_pattern = to_abs(yml["paths"]["report_file"])

        config.tests.append(test)

    return config

# ==================================================================================================
# Logging.


class Style:
    """
    An enum-like class for storing different ANSI color codes.
    """
    TITLE = "\033[1;34m"
    PREAMBLE = "\033[34m"
    TASK_HEADING = "\033[1;34m"
    TASK_SUBHEADING = "\033[34m"
    SUMMARY = "\033[34m"
    SEPARATOR_LINE = "\033[34m"
    EXCEPTION = "\033[35m"
    ERROR = "\033[31m"
    WARN = "\033[33m"
    SUCCESS = "\033[32m"
    BOLD = "\033[1m"
    ITALIC = "\033[3m"
    NONE = "\033[0m"


def sprintln(s: str = "", style: Style = None):
    """
    Prints the given string in the given style to stdout. Terminates the string with a newline char.
    """
    return sprint(s, style=style, suffix="\n", flush=True)


def sprint(s: str, style: Style = None, suffix: str = "", flush: bool = False):
    """
    Prints the given string in the given style to stdout. The given {suffix} is appended to the
    string. Flushes the write buffer if {flush} is set to True.
    """
    if style:
        sys.stdout.write(style)
    sys.stdout.write(f"{s}{suffix}")
    if style:
        sys.stdout.write(Style.NONE)
    if flush:
        sys.stdout.flush()


def format(s: str, **kwargs):
    """
    Replaces each placeholder of form '{key}' in the given string with the value associated with
    the key in kwargs. If kwargs does not contain such key/value pair, the placeholder remains
    unreplaced. For example, if s = "Hello {name1} and {name2}", calling 'format(s, name1="Bob")'
    returns "Hello Bob and {name2}".
    """
    for key, value in kwargs.items():
        s = s.replace(f"{{{str(key)}}}", str(value))
    return s

# ==================================================================================================


def wdiff(from_path: Path, to_path: Path) -> tuple[str, int, int]:
    """
    Compares the contents of {from_path} and {to_path} using the "word-diff" mode of git diff.

    Args:
        from_path:
            The path to the first of the two files to compare.
        to_path:
            The path to the second of the two files to compare.

    Returns:
        A tuple (diff_result, num_inserts, num_deletes), where {diff_result} is text in "diff"
        format representing the changes between the two files, {num_inserts} is the number of
        insertions and {num_deletes} the number of deletions necessary to transform {from_path} to
        {to_path}.
    """
    # Compute the diff result using git diff.
    wdiff_cmd = f"git --no-pager diff --word-diff --no-index \"{from_path}\" \"{to_path}\""
    status, wdiff_output, stderr = exec_command(wdiff_cmd)

    if stderr:
        raise ValueError(f"{stderr} ({status})")

    # Compute the number of inserts and deletes necessary to transform {from_path} to {to_path}.
    wdiff_stat_cmd = f"{wdiff_cmd} --numstat"
    status, wdiff_stat_output, stderr = exec_command(wdiff_stat_cmd)

    if stderr:
        raise ValueError(f"{stderr} ({status})")

    # The expected format of wdiff_stat_output is: "<inserts>\t<deletes>\tfrom_path => to_path".
    wdiff_stat_fields = wdiff_stat_output.split("\t") if wdiff_stat_output else []
    inserts = int(wdiff_stat_fields[0]) if len(wdiff_stat_fields) == 3 else 0
    deletes = int(wdiff_stat_fields[1]) if len(wdiff_stat_fields) == 3 else 0

    return wdiff_output, inserts, deletes


# ==================================================================================================

def exec_command(cmd: str, strip_output: bool = False) -> tuple[int, str, str]:
    """
    Executes the given shell command.

    Args:
        cmd:
            The command to execute.
        strip_output:
            Whether to strip leading and trailing whitespaces from the output produced by {cmd}.

    Returns:
        A tuple (status, stdout, stderr) where {status} is the exit code returned by {cmd},
        {stdout} is the output printed to stdout, and {stderr} the output printed to stderr.
    """
    p = subprocess.run(cmd, capture_output=True, text=True, shell=True)
    return p.returncode, \
        p.stdout.strip() if strip_output and p.stdout else p.stdout, \
        p.stderr.strip() if strip_output and p.stderr else p.stderr
