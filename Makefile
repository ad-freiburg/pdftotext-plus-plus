# --------------------------------------------------------------------------------------------------
# General metadata.

PROJECT_NAME = pdftotext-plus-plus
PROGRAM_NAME = pdftotext++
MAINTAINER_NAME = Claudius Korzen
MAINTAINER_MAIL = korzen@cs.uni-freiburg.de
DESCRIPTION = $(shell cat project.description | sed ':a;N;$$!ba;s/\n/\\n/g')  # the sed command replaces each newline by "\n".
USAGE = $(shell cat project.usage | sed ':a;N;$$!ba;s/\n/\\n/g')  # the sed command replaces each newline by "\n".
VERSION = $(shell cat project.version)

# --------------------------------------------------------------------------------------------------
# Paths to important files and directories.

SRC_DIR = src
UNIT_TEST_DIR = test
E2E_TEST_DIR = e2e
RESOURCES_DIR = resources
USR_DIR = usr
BUILD_DIR = build

MAIN_CPP_FILE = src/pdftotext++.cpp
MAIN_BINARY = $(basename $(notdir $(MAIN_CPP_FILE)))
MAIN_OBJECT_FILE = $(MAIN_CPP_FILE:%.cpp=$(BUILD_DIR)/%.o)

SRC_HEADER_FILES = $(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/**/*.h)
SRC_CPP_FILES = $(filter-out %$(MAIN_CPP_FILE), $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/**/*.cpp))
SRC_OBJECT_FILES = $(SRC_CPP_FILES:%.cpp=$(BUILD_DIR)/%.o)

UNIT_TEST_CPP_FILES = $(wildcard $(UNIT_TEST_DIR)/*Test.cpp $(UNIT_TEST_DIR)/**/*Test.cpp)
UNIT_TEST_BINARIES = $(basename $(UNIT_TEST_CPP_FILES:%.cpp=$(BUILD_DIR)/%.o))

SEMANTIC_ROLES_DETECTION_MODELS_DIR=$(RESOURCES_DIR)/models/2021-08-30_model-3K-documents

# --------------------------------------------------------------------------------------------------
# Settings related to packaging.

PACKAGING_DOCKERFILES_DIR = Dockerfiles.packaging
PACKAGING_TARGET_DIR = /local/data/pdftotext-plus-plus/packages
PACKAGING_APT_PACKAGES_RUN = libboost-all-dev, libfontconfig1-dev, libfreetype6-dev, libnss3-dev, libopenjp2-7-dev, libtiff5-dev

# --------------------------------------------------------------------------------------------------
# Settings related to the APT repository.

APT_REPO_DOCKERFILE = services/apt-repo/Dockerfile
APT_REPO_IMAGE_NAME = $(PROJECT_NAME).apt-repo
APT_REPO_SERVER_CONTAINER_NAME = $(PROJECT_NAME).apt-repo.server
APT_REPO_UPDATE_CONTAINER_NAME = $(PROJECT_NAME).apt-repo.update
APT_REPO_POOL_DIR = $(PACKAGING_TARGET_DIR)
APT_REPO_DISTS_DIR = /local/data/pdftotext-plus-plus/apt-repo/dists
APT_REPO_GPG_DIR = /local/data/pdftotext-plus-plus/keyrings/apt-repo
APT_REPO_SERVER_PORT = 7192

# --------------------------------------------------------------------------------------------------
# Settings related to logging.

INFO_STYLE = \033[34;1m
ERROR_STYLE = \033[31;1m
N = \033[0m

# --------------------------------------------------------------------------------------------------
# Settings related to CXX.

CXX_MACROS      = -DCONFIG_SEMANTIC_ROLES_DETECTION_MODELS_DIR="\"$(SEMANTIC_ROLES_DETECTION_MODELS_DIR)\""
CXX_MACROS_MAIN = -DPROGRAM_NAME="\"$(PROGRAM_NAME)\"" -DPROGRAM_DESCRIPTION="\"$(DESCRIPTION)\"" -DPROGRAM_USAGE="\"$(USAGE)\"" -DVERSION="\"$(VERSION)\""
CXX_LIBS        = -I$(USR_DIR)/include -L$(USR_DIR)/lib -ltensorflow_framework -ltensorflow -lpoppler -lutf8proc -lboost_program_options
CXX_LIBS_TEST   = $(CXX_LIBS) -lgtest -lgtest_main -lpthread
CXX_DEBUG       = g++ -std=c++20 -Wall -g $(CXX_MACROS)
CXX_PROD        = g++ -std=c++20 -Wall -O3 $(CXX_MACROS)
CXX_TEST        = g++ -std=c++20 -Wall -g $(CXX_MACROS)
CXX							= $(CXX_PROD)

# ==================================================================================================

.PHONY: help checkstyle compile-debug compile-prod compile test unit-test e2e-test \
  e2e-analyze-latest e2e-analyze-vscode install install-without-requirements clean release \
	check-version set-version version packages apt-repo apt-repo/build-docker-image \
	apt-repo/update apt-repo/server/start apt-repo/server/start/force apt-repo/server/stop \
	requirements/checkstyle requirements/compile requirements/test requirements/install \
	requirements/packages

# --------------------------------------------------------------------------------------------------
# Help.

help:
	@echo TODO

# --------------------------------------------------------------------------------------------------
# Checkstyle.

checkstyle:
	@echo "$(INFO_STYLE)[$@] Checking style of header files ...$(N)"
	python3 cpplint.py --repository="$(SRC_DIR)" "$(SRC_DIR)"/*.h "$(SRC_DIR)"/**/*.h

	@echo "$(INFO_STYLE)[$@] Checking style of cpp files ...$(N)"
	python3 cpplint.py --repository="$(SRC_DIR)" "$(SRC_DIR)"/*.cpp "$(SRC_DIR)"/**/*.cpp

	@echo "$(INFO_STYLE)[$@] Checking style of test files ...$(N)"
	python3 cpplint.py --repository="$(UNIT_TEST_DIR)" "$(UNIT_TEST_DIR)"/*.cpp "$(UNIT_TEST_DIR)"/**/*.cpp

# --------------------------------------------------------------------------------------------------
# Compiling.

compile-debug: CXX = $(CXX_DEBUG)
compile-debug: compile

compile-prod: CXX = $(CXX_PROD)
compile-prod: compile

compile: $(MAIN_BINARY)

$(MAIN_BINARY): $(MAIN_OBJECT_FILE) $(SRC_OBJECT_FILES)
	@echo "$(INFO_STYLE)[compile] Creating main binary '$@' ...$(N)"
	@mkdir -p "$(dir $(BUILD_DIR)/$@)"
	$(CXX) -o "$(BUILD_DIR)/$@" $^ $(CXX_LIBS)

$(BUILD_DIR)/%.o: %.cpp $(SRC_HEADER_FILES)
	@echo "$(INFO_STYLE)[compile] Compiling '$<' ...$(N)"
	@mkdir -p "$(dir $@)"
	if [ "$<" = "$(MAIN_CPP_FILE)" ]; then\
		$(CXX) -c $< -o "$@" $(CXX_MACROS_MAIN) $(CXX_LIBS);\
	else\
		$(CXX) -c $< -o "$@" $(CXX_LIBS);\
  fi

# --------------------------------------------------------------------------------------------------
# Testing.

test: unit-test e2e-test

unit-test: $(UNIT_TEST_BINARIES)
	@for T in $(UNIT_TEST_BINARIES); do \
		echo "$(INFO_STYLE)[$@] Running unit test '$$T' ...$(N)" ; \
    export TF_CPP_MIN_LOG_LEVEL=3 ; \
		./$$T || exit; \
	done

%Test: %Test.o $(SRC_OBJECT_FILES)
	@echo "$(INFO_STYLE)[test] Creating unit test '$@' ...$(N)"
	@mkdir -p "$(dir $@)"
	$(CXX_TEST) -o "$@" $^ $(CXX_LIBS_TEST)

$(BUILD_DIR)/%Test.o: %Test.cpp $(SRC_HEADER_FILES)
	@echo "$(INFO_STYLE)[test] Compiling '$<' ...$(N)"
	@mkdir -p "$(dir $@)"
	$(CXX_TEST) -c $< -o "$@" $(CXX_LIBS_TEST)

e2e-test: $(MAIN_BINARY)
	@python3 $(E2E_TEST_DIR)/e2e.py run --ppp $(abspath $(BUILD_DIR)/$(MAIN_BINARY))

e2e-analyze:
	@python3 $(E2E_TEST_DIR)/e2e.py analyze --dir "[latest-test-result-dir]"

e2e-analyze-in-vscode:
	@python3 $(E2E_TEST_DIR)/e2e.py analyze --dir "[latest-test-result-dir]" --vscode

e2e-test-pdfs:
	cd $(E2E_TEST_DIR)/pdfs && latexmk *.tex && latexmk -c

# --------------------------------------------------------------------------------------------------
# Installing.

install: requirements/install install-without-requirements

install-without-requirements: clean compile-prod
	@echo "$(INFO_STYLE)[$@] Installing pdftotext++ ...$(N)"
	mkdir -p "/usr/lib/$(PROJECT_NAME)"
	cp -Pa "$(USR_DIR)/lib/." "/usr/lib/$(PROJECT_NAME)"
	ldconfig "/usr/lib/$(PROJECT_NAME)"
	mkdir -p "/usr/share/$(PROJECT_NAME)/resources"
	cp -Pa "$(RESOURCES_DIR)/." "/usr/share/$(PROJECT_NAME)/resources"
	cp "$(BUILD_DIR)/$(MAIN_BINARY)" "/usr/bin/$(MAIN_BINARY)"
	chmod +x "/usr/bin/$(MAIN_BINARY)"

# --------------------------------------------------------------------------------------------------
# Cleaning.

clean:
	@echo "$(INFO_STYLE)[$@] Cleaning the project ...$(N)"
	rm -f core
	rm -rf $(BUILD_DIR)
	rm -f $(UNIT_TEST_DIR)/*.aux $(UNIT_TEST_DIR)/**/*.aux
	rm -f $(UNIT_TEST_DIR)/*.fdb_latexmk $(UNIT_TEST_DIR)/**/*.fdb_latexmk
	rm -f $(UNIT_TEST_DIR)/*.fls $(UNIT_TEST_DIR)/**/*.fls
	rm -f $(UNIT_TEST_DIR)/*.log $(UNIT_TEST_DIR)/**/*.log
	rm -f $(UNIT_TEST_DIR)/*.synctex.gz $(UNIT_TEST_DIR)/**/*.synctex.gz
	rm -f $(UNIT_TEST_DIR)/*.bbl $(UNIT_TEST_DIR)/**/*.bbl
	rm -f $(UNIT_TEST_DIR)/*.blg $(UNIT_TEST_DIR)/**/*.blg
	rm -f $(E2E_TEST_DIR)/*.aux $(E2E_TEST_DIR)/**/*.aux
	rm -f $(E2E_TEST_DIR)/*.fdb_latexmk $(E2E_TEST_DIR)/**/*.fdb_latexmk
	rm -f $(E2E_TEST_DIR)/*.fls $(E2E_TEST_DIR)/**/*.fls
	rm -f $(E2E_TEST_DIR)/*.log $(E2E_TEST_DIR)/**/*.log
	rm -f $(E2E_TEST_DIR)/*.synctex.gz $(E2E_TEST_DIR)/**/*.synctex.gz
	rm -f $(E2E_TEST_DIR)/*.bbl $(E2E_TEST_DIR)/**/*.bbl
	rm -f $(E2E_TEST_DIR)/*.blg $(E2E_TEST_DIR)/**/*.blg

# --------------------------------------------------------------------------------------------------
# Releasing.

release: check-version packages apt-repo github-release set-version

check-version:
	@if [ "$(VERSION)" = "$(shell cat project.version)" ]; then \
		echo "$(ERROR_STYLE)The specified version is equal to the current version ($(VERSION))." ; \
		echo "$(ERROR_STYLE)Specify another version with: make release VERSION=\"<version>\"$(N)" ; \
		exit 1 ; \
	fi

set-version:
	@echo "$(VERSION)" > project.version

version:
	@cat project.version

packages:
	@echo "$(INFO_STYLE)[release] Building packages ...$(N)"
	./scripts/packaging.sh build_packages "$(PACKAGING_DOCKERFILES_DIR)" "$(PROJECT_NAME)" \
			"$(PROGRAM_NAME)" "$(DESCRIPTION)" "$(VERSION)" "$(MAINTAINER_NAME)" \
			"$(MAINTAINER_MAIL)" "$(PACKAGING_APT_PACKAGES_RUN)" "$(PACKAGING_TARGET_DIR)"

# --------------------------------------------------------------------------------------------------
# APT repository.

apt-repo: apt-repo/update apt-repo/server/start

apt-repo/build-docker-image:
	@echo "$(INFO_STYLE)[apt-repo] Building a Docker image from '$(APT_REPO_DOCKERFILE)' ...$(N)"
	cd "$(dir $(APT_REPO_DOCKERFILE))" && \
	docker build -f "$(notdir $(APT_REPO_DOCKERFILE))" -t "$(APT_REPO_IMAGE_NAME)" .

apt-repo/update: apt-repo/build-docker-image
	@echo "$(INFO_STYLE)[apt-repo] Updating the APT repository ...$(N)"
	docker run --rm -it \
		  --name "$(APT_REPO_UPDATE_CONTAINER_NAME)" \
		  -v "$(APT_REPO_POOL_DIR)":/repo/pool \
		  -v "$(APT_REPO_DISTS_DIR)":/repo/dists \
		  -v "$(APT_REPO_GPG_DIR)":/gpg \
		  "$(APT_REPO_IMAGE_NAME)" update

apt-repo/server/start: apt-repo/build-docker-image
	@echo "$(INFO_STYLE)[apt-repo] Starting the APT repository server ...$(N)"
	@if [ "$(shell docker ps -f 'name=$(APT_REPO_SERVER_CONTAINER_NAME)' --format '{{.Names}}')" = \
			"$(APT_REPO_SERVER_CONTAINER_NAME)" ]; then \
    echo "Server is already running."; \
		echo "To force restarting the server, type 'make apt-repo/server/start/force'"; \
	else \
		docker run --rm -it -d \
			--name "$(APT_REPO_SERVER_CONTAINER_NAME)" \
			-p "$(APT_REPO_SERVER_PORT)":80 \
			-v "$(APT_REPO_POOL_DIR)":/repo/pool \
			-v "$(APT_REPO_DISTS_DIR)":/repo/dists \
			-v "$(APT_REPO_GPG_DIR)":/gpg \
			"$(APT_REPO_IMAGE_NAME)" start_server ; \
	fi

apt-repo/server/start/force: apt-repo/server/stop apt-repo/server/start

apt-repo/server/stop:
	@echo "$(INFO_STYLE)[apt-repo] Stopping the APT repository server ...$(N)"
	docker stop "$(APT_REPO_SERVER_CONTAINER_NAME)"

# ==================================================================================================
# Github release.

github-release:
	@echo "$(INFO_STYLE)[github-release] Creating Github release ...$(N)"
	./services/github-release/github-release.sh create_release "$(VERSION)" "$(PACKAGING_TARGET_DIR)"

# ==================================================================================================
# Installing requirements.

requirements/checkstyle:
	@echo "$(INFO_STYLE)[$@] Installing requirements for 'make checkstyle' ...$(N)"
	./scripts/install_requirements.sh make_checkstyle $(USR_DIR)

requirements/compile:
	@echo "$(INFO_STYLE)[$@] Installing requirements for 'make compile' ...$(N)"
	./scripts/install_requirements.sh make_compile $(USR_DIR)

requirements/test: requirements/unit-test requirements/e2e-test

requirements/unit-test:
	@echo "$(INFO_STYLE)[$@] Installing requirements for 'make unit-test' ...$(N)"
	./scripts/install_requirements.sh make_unit_test $(USR_DIR)

requirements/e2e-test:
	@echo "$(INFO_STYLE)[$@] Installing requirements for 'make e2e-test' ...$(N)"
	./scripts/install_requirements.sh make_e2e_test $(USR_DIR)

requirements/install:
	@echo "$(INFO_STYLE)[$@] Installing requirements for 'make install' ...$(N)"
	./scripts/install_requirements.sh make_install $(USR_DIR)

requirements/packages:
	@echo "$(INFO_STYLE)[$@] Installing requirements for 'make packages' ...$(N)"
	./scripts/install_requirements.sh make_packages $(USR_DIR)
