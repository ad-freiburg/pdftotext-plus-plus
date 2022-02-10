PROJECT_NAME = pdftotext-plus-plus
INSTALL_DIR = /usr/local

help:
	@echo "XXX"

install: build-docker
	docker create -it --name $(PROJECT_NAME) $(PROJECT_NAME) bash
	docker cp $(PROJECT_NAME):/usr/local/lib $(INSTALL_DIR)/
	docker cp $(PROJECT_NAME):/usr/local/bin $(INSTALL_DIR)/
	docker rm $(PROJECT_NAME)

build-docker:
	docker build -t $(PROJECT_NAME) .