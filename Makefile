cmd 			?= RUN
args			?=

.PHONY: all

all: docker

# TODO make a first run command? or if the cmd is provided to rebuild it
docker:
	@docker build -t numero_uri --target ${cmd} .
	-@docker run -v ${PWD}:/numero_uri -e in_args="${args}" \
		-it numero_uri

docker-run:
	-@docker run -v ${PWD}:/numero_uri -e in_args="${args}" \
		-it numero_uri

docker-test:
	@docker build -t numero_uri --target test .
	-@docker run -v ${PWD}:/numero_uri -e \
		-it numero_uri

build:
	cmake -B build && cmake --build build

test:
	cmake -B build -DBUILD_TESTS=ON && cmake --build build \
		&& ctest --test-dir build/tests --output-on-failure

clean:
	rm -rf build