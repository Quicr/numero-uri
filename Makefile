cmd 			?= RUN
args			?=

.PHONY: all

all: docker

docker:
	@docker build -t numero_uri --target ${cmd} .
	-@docker run -v ${PWD}:/numero_uri -e in_args="${args}" \
		--rm -it numero_uri

build:
	cmake -B build && cmake --build build

test:
	cmake -B build -DBUILD_TESTS=ON && cmake --build build \
		&& ctest --test-dir build/tests --output-on-failure

clean:
	rm -rf build