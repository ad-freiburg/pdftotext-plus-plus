PROJECT_NAME = pdftotext-plus-plus

INPUT_DIR = /local/data/pdftotext++/evaluation/benchmarks/arxiv-debug/
OUTPUT_DIR = /local/data/pdftotext++/evaluation/extraction-results/arxiv-debug
SRC_DIR = src
LIB_DIR = lib
POPPLER_DIR = ../poppler

CPPLINT_FILE = cpplint.py
MAIN_BINARY_FILES = $(basename $(wildcard $(SRC_DIR)/*Main.cpp $(SRC_DIR)/*/*Main.cpp))
TEST_BINARY_FILES = $(basename $(wildcard $(SRC_DIR)/*Test.cpp $(SRC_DIR)/*/*Test.cpp))
HEADER_FILES = $(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/*/*.h)
OBJECT_FILES = $(addsuffix .o, $(basename $(filter-out %Main.cpp %Test.cpp, $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/*/*.cpp))))

#CXX = g++ -g -Wall -std=c++17
CXX = g++ -O3 -Wall -std=c++17
LIBS = -I$(POPPLER_DIR) -I$(POPPLER_DIR)/build -I$(POPPLER_DIR)/build/poppler -L $(LIB_DIR) -lpoppler -lpoppler-cpp -ltensorflow_cc -ltensorflow_framework -lutf8proc
LIBS_TEST = $(LIBS) -lgtest -lgtest_main -lpthread

DOCKER_CMD = docker

# ==================================================================================================

help:
	@echo "\033[34;1mDESCRIPTION\033[0m"
	@echo "    This is the Makefile of pdftotext++."
	@echo
	@echo "\033[34;1mTARGETS\033[0m"
	@echo "    \033[1mdataset\033[0m"
	@echo "        Creates a dataset containing semantic and layout information about the words and"
	@echo "        text blocks extracted from given PDF files."
	@echo
	@echo "    For more information about a target, type \"make help-<target>\"."
	@echo

# ==================================================================================================
# make dataset

help-dataset:
	@echo "\033[34;1mNAME\033[0m"
	@echo "    dataset"
	@echo
	@echo "\033[34;1mDESCRIPTION\033[0m"
	@echo "    Creates a dataset containing semantic and layout information about the words *and*"
	@echo "    text blocks extracted from given PDF files (= files ending with "*.pdf"). The"
	@echo "    information about the words extracted from (INPUT_DIR)/path/to/file.pdf will be"
	@echo "    written to (INPUT_DIR)/path/to/file.words.jsonl; the information about the text"
	@echo "    blocks will be written to (INPUT_DIR)/path/to/file.blocks.jsonl. The files will"
	@echo "    contain one word (resp. text block) per line. A line of file.words.jsonl is of the"
	@echo "    following form:"
	@echo
	@echo "    {\"id\": \"w-ynXLvp8F\", \"rank\": 0, \"page\": 1, \"minX\": 177.797, \"minY\": 70.6,"
	@echo "     \"maxX\": 236.4, \"maxY\": 86.1, \"font\": \"MOFJOH+NimRomNo9L-Medi\", \"fontSize\":"
	@echo "     11.9, \"text\": \"Topological\", \"block\": \"tb-IztYpXWG\", \"origin\": \"pdf\"}"
	@echo
	@echo "    A line of file.blocks.jsonl is of the following form:"
	@echo
	@echo "    {\"id\": \"tb-IztYpXWG\", \"rank\": 0, \"page\": 1, \"minX\": 177.7, \"minY\": 70.6,"
	@echo "     \"maxX\": 432.4, \"maxY\": 86.1, \"font\": \"MOFJOH+NimRomNo9L-Medi\", \"fontSize\":"
	@echo "     11.9, \"text\": \"Topological ...\", \"role\": \"title\", \"origin\": \"pdf\"}"
	@echo
	@echo "\033[34;1mUSAGE\033[0m"
	@echo "    make dataset [INPUT_DIR=\033[4mstring\033[0m] [OUTPUT_DIR=\033[4mstring\033[0m]"
	@echo
	@echo "\033[34;1mOPTIONS\033[0m"
	@echo "    \033[1mINPUT_DIR\033[0m (default: \"$(INPUT_DIR)\")"
	@echo "        The path to a directory containing PDF files (= files ending with *.pdf), stored"
	@echo "        in arbitrary depth."
	@echo "    \033[1mOUTPUT_DIR\033[0m (default: \"$(OUTPUT_DIR)\")"
	@echo "        The path to the directory to which the *.words.jsonl and *.blocks.jsonl files"
	@echo "        should be written. The produced directory structure is equal to the input"
	@echo "        directory structure. For example, when the input directory structure is"
	@echo
	@echo "        INPUT_DIR/"
	@echo "          0001/"
	@echo "            math-ph0001044/"
	@echo "              math-ph0001044.pdf"
	@echo "          0002/"
	@echo "            physics0002056/"
	@echo "              physics0002056.pdf"
	@echo
	@echo "        the following directory structure is produced:"
	@echo
	@echo "        OUTPUT_DIR/"
	@echo "          0001/"
	@echo "            math-ph0001044/"
	@echo "              math-ph0001044.words.jsonl"
	@echo "              math-ph0001044.blocks.jsonl"
	@echo "          0002/"
	@echo "            physics0002056/"
	@echo "              physics0002056.words.jsonl"
	@echo "              physics0002056.blocks.jsonl"
	@echo

dataset:
	@echo "\033[34;1mCreating output directory ...\033[0m"
	mkdir -p -m 777 $(OUTPUT_DIR)

	@echo "\033[34;1mRunning the Docker container ...\033[0m"
	cd ${INPUT_DIR} && find . -name "*.pdf" | sed "s/.pdf$$//" | xargs -I {} docker run --rm -v ${INPUT_DIR}:/input -v ${OUTPUT_DIR}:/output pdftotext-plus-plus /input/{}.pdf --serialize-words "/output/{}.words.jsonl" --serialize-text-blocks "/output/{}.blocks.jsonl" "/output/{}.txt"

# ==================================================================================================

build-docker:
	@echo "\033[34;1mBuilding the Docker image ...\033[0m"

	$(DOCKER_CMD) build -t $(PROJECT_NAME) .

# ==================================================================================================

.PRECIOUS: %.o

all: compile test checkstyle

# compile: $(MAIN_BINARY_FILES) $(TEST_BINARY_FILES)
compile: $(MAIN_BINARY_FILES)

test: $(TEST_BINARY_FILES)
	for T in $(TEST_BINARY_FILES); do ./$$T; done

checkstyle:
	@# Filter HEADER_FILES_guards and include check, doesn't work well for
	@# projects roots that are not a svn or git root.
	@# Allow non-const references.
	python3 $(CPPLINT_PATH) --repository=. --filter='$(CPPLINT_FILTERS)' *.h *.cpp

clean:
	rm -f **/*.o
	rm -f $(MAIN_BINARY_FILES)
	rm -f $(TEST_BINARY_FILES)
	rm -f core

%Main: %Main.o $(OBJECT_FILES)
	$(CXX) -o $@ $^ $(LIBS)

%Test: %Test.o $(OBJECT_FILES)
	$(CXX) -o $@ $^ $(LIBS_TEST)

%.o: %.cpp $(HEADER_FILES)
	$(CXX) -c $< -o $@ $(LIBS)
