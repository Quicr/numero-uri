.PHONY: all

all: docker

docker:
	@docker build -t webex_http_encoder .

	@echo ""
	@echo "--Run Executable--"
	@echo ""
	-@docker run --name webex_http_encoder -it webex_http_encoder

	@echo ""
	@echo "--Clean up container--"
	docker rm webex_http_encoder
