NAME := ip_camera_api
FOLDERS := ./app/ ./tests/

.DEFAULT_GOAL := help


.PHONY: help
help:
	@echo "Available make commands for $(NAME):"
	@echo ""
	@echo "  install     install packages and prepare environment"
	@echo "  clean       remove all temporary files"
	@echo "  lint        run the code linters"
	@echo "  format      reformat code"
	@echo "  test        run all the tests"
	@echo "  coverage    run tests and generate coverage report"
	@echo "  console     start python cli with env vars set"
	@echo "  dev         start uvicorn development server"
	@echo "  server      server in production mode"
	@echo ""


.PHONY: install
install:
	mkdir -p python_env; \
	python3 -m venv python_env; \
	. python_env/bin/activate; \
	python3 -m pip install --upgrade pip; \
	python3 -m pip install -r requirements.txt; \
	python3 -m pip install -r requirements-test.txt


.PHONY: clean
clean:
	find . -type d -name "__pycache__" | xargs rm -rf {}; \
	rm -rf .coverage htmlcov


.PHONY: lint
lint:
	@. python_env/bin/activate; \
	pycodestyle --max-line-length=120 --exclude=.git,python_env,__pycache__  . && echo "pycodestyle OK."

.PHONY: format
format:
	@. python_env/bin/activate; \
	autopep8 --in-place -r app; \
	autopep8 --in-place -r tests;

.PHONY: test
test:
	@. python_env/bin/activate; \
	python -m pytest

.PHONY: coverage
coverage:
	@. python_env/bin/activate; \
	python -m pytest --cov-config=.coveragerc --cov . .  --cov-report html --cov-report term

.PHONY: console
console:
	@. python_env/bin/activate; \
	python

.PHONY: dev 
dev:
	@. python_env/bin/activate; \
	uvicorn app.server:app --reload --port 3000 --no-access-log --reload-dir app

.PHONY: server
server:
	@. python_env/bin/activate; \
	python main.py
