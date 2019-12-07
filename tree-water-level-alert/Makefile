GREEN  := $(shell tput -Txterm setaf 2)
YELLOW := $(shell tput -Txterm setaf 3)
BLUE   := $(shell tput -Txterm setaf 4)
WHITE  := $(shell tput -Txterm setaf 7)
NC     := $(shell tput -Txterm sgr0)

.PHONY: help
help: ## Print this help messag
	@awk -F ':|##' '/^[^\t].+?:.*?##/ { printf "${BLUE}%-20s${NC}%s\n", $$1, $$NF }' $(MAKEFILE_LIST) | \
    sort

ide-update: ## Update IDE sources
	platformio init --board nodemcuv2 --ide clion
clean: ## clean up artifacts
	platformio run -t clean
build: ## build the project
	platformio run
upload: ## upload to device
	platformio run -t upload
monitor: ## monitor serial output
	platformio serialports monitor
