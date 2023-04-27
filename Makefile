cmd 			?= RUN
args			?=

CLANG_FORMAT=clang-format -i

.PHONY: all

all: docker

# TODO make a first run command? or if the cmd is provided to rebuild it
docker:
	@docker build -t numero_uri --target ${cmd} .
	-@docker run -v ${PWD}:/numero_uri -e in_args="${args}" \
		--rm -it numero_uri

docker-run:
	-@docker run -v ${PWD}:/numero_uri -e in_args="${args}" \
		--rm -it numero_uri

docker-test:
	@docker build -t numero_uri --target test .
	-@docker run -v ${PWD}:/numero_uri --rm -it numero_uri

build:
	cmake -B build && cmake --build build

test:
	cmake -B build -DBUILD_TESTING=ON -DNUMERO_URI_BUILD_TESTS=ON \
		&& cmake --build build \
		&& ctest --test-dir build/tests --output-on-failure

clean:
	rm -rf build

format:
	@find lib/inc -iname "*.h" -or -iname "*.cpp" -or -iname "*.cc" | xargs ${CLANG_FORMAT}
	@find lib/src -iname "*.h" -or -iname "*.cpp" -or -iname "*.cc" | xargs ${CLANG_FORMAT}
	@find tests -iname "*.h" -or -iname "*.cpp" -or -iname "*.cc" | xargs ${CLANG_FORMAT}
