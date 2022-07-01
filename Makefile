PROJECT_NAME = pdftotext-plus-plus

DOCKER_CMD = docker
DOCKER_FILE = ./Dockerfiles/Dockerfile.ubuntu:$(shell lsb_release -r -s)
DOCKER_IMAGE = pdftotext-plus-plus-ubuntu:$(shell lsb_release -r -s)

INPUT_DIR = /local/data/pdftotext-plus-plus/evaluation/benchmarks/arxiv-debug
OUTPUT_DIR = /local/data/pdftotext-plus-plus/evaluation/extraction-results/arxiv-debug

INSTALL_DIR = /usr/local

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
	@echo "    {\"id\": \"w-ynXLvp8F\", \"rank\": 0, \"page\": 1, \"leftX\": 177.797, \"upperY\": 70.6,"
	@echo "     \"rightX\": 236.4, \"lowerY\": 86.1, \"font\": \"MOFJOH+NimRomNo9L-Medi\", \"fontSize\":"
	@echo "     11.9, \"text\": \"Topological\", \"block\": \"tb-IztYpXWG\", \"origin\": \"pdf\"}"
	@echo
	@echo "    A line of file.blocks.jsonl is of the following form:"
	@echo
	@echo "    {\"id\": \"tb-IztYpXWG\", \"rank\": 0, \"page\": 1, \"leftX\": 177.7, \"upperY\": 70.6,"
	@echo "     \"rightX\": 432.4, \"lowerY\": 86.1, \"font\": \"MOFJOH+NimRomNo9L-Medi\", \"fontSize\":"
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

dataset: build-docker
	@echo "\033[34;1mCreating output directory ...\033[0m"
	mkdir -p -m 777 $(OUTPUT_DIR)

	echo "\033[34;1mRunning the Docker container ...\033[0m"
	cd ${INPUT_DIR} && find . -name "*.pdf" | sed "s/.wc.pdf$$//" | xargs -I {} docker run --rm -v ${INPUT_DIR}:/input -v ${OUTPUT_DIR}:/output $(DOCKER_IMAGE) /input/{}.wc.pdf --output-pages --output-text-blocks --output-words "/output/{}.jsonl"
	echo "{ \"benchmarkName\": \"$(shell basename $(INPUT_DIR))\" }" > ${OUTPUT_DIR}/info.json

# ==================================================================================================

build-docker:
	@echo "\033[34;1mBuilding the Docker image ...\033[0m"

#$(DOCKER_CMD) build -f $(DOCKER_FILE) -t $(DOCKER_IMAGE) .
	$(DOCKER_CMD) build -f Dockerfile.test -t pdftotext-plus-plus-test .

# ==================================================================================================

compile: build-docker
	@echo "\033[34;1mCompiling pdftotext++ ...\033[0m"
	$(DOCKER_CMD) run --rm pdftotext-plus-plus-test

test: build-docker
	@echo "\033[34;1mTesting pdftotext++ ...\033[0m"
	$(DOCKER_CMD) run --rm --entrypoint make $(DOCKER_IMAGE) test

checkstyle: build-docker
	@echo "\033[34;1mCheckstyling pdftotext++ ...\033[0m"
	$(DOCKER_CMD) run --rm pdftotext-plus-plus-test

# ==================================================================================================

install: build-docker
	@echo "\033[34;1mInstalling pdftotext++ ...\033[0m"

	$(DOCKER_CMD) create -it --name $(PROJECT_NAME)-install $(DOCKER_IMAGE) bash
	$(DOCKER_CMD) cp $(PROJECT_NAME)-install:/usr/local/lib $(INSTALL_DIR)/
	$(DOCKER_CMD) cp $(PROJECT_NAME)-install:/usr/local/bin $(INSTALL_DIR)/
	$(DOCKER_CMD) rm $(PROJECT_NAME)-install

