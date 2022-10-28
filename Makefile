TARGET_NAME			= webex_http_encoder

CC					= g++

SRC 				+= $(shell find src -name '*.cc' -exec basename {} \;)
INCS 				+= $(addprefix -I,$(shell find inc -type d))

CFLAGS 				= -Wall -g -std=c++17 -Os
CFLAGS 				+= $(INCS)

VPATH 				= $(shell find src -type d)

BUILD_DIR 			= build
OBJ_DIR				= ${BUILD_DIR}/obj
DEP_DIR				= ${BUILD_DIR}/dep
BIN_DIR				= ${BUILD_DIR}/bin
TARGET				= ${BIN_DIR}/${TARGET_NAME}

OBJS 				= ${addprefix ${OBJ_DIR}/,${SRC:.cc=.o}}
DEPS				= ${addprefix ${DEP_DIR}/,${SRC:.cc=.d}}

.PHONY: all

all: docker

-include ${DEPS}

build: ${TARGET}

dirs: ${OBJ_DIR} ${DEP_DIR} ${BIN_DIR}
${OBJ_DIR} ${DEP_DIR} ${BIN_DIR}:
	@echo "[MKDIR] $@"
	@mkdir -p $@

${OBJ_DIR}/%.o : %.cc | dirs
	@echo "[CC]			${notdir $<}"
	@${CC} ${CFLAGS} -c -o $@ $< -MMD -MF ${DEP_DIR}/${*F}.d

${TARGET}: ${OBJS}
	@echo "[COMPILE]	${TARGET}"
	@${CC} ${CFLAGS} $^ -o $@

docker:
	@docker build -t webex_http_encoder .

	@echo ""
	@echo "--Run Executable--"
	@echo ""
	-@docker run -v ${PWD}:/webex_http_encoder --name webex_http_encoder -it webex_http_encoder

	@echo ""
	@echo "--Clean up container--"
	docker rm webex_http_encoder


