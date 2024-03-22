SHELL = /bin/bash
PYTHON ?= python

envDir = venv
envPrompt ?= "pod5"

.PHONY: clean install update docs

# Clean the python virtual environment
clean:
	rm -rf ${envDir}

# Completely install the python environment for development
install: clean
	${PYTHON} -m venv --prompt=${envPrompt} ${envDir}
	source ${envDir}/bin/activate \
	&& pip install --upgrade pip \
	&& pip install -e .[dev] \
	&& pre-commit install \
	&& pip install --editable ../../../src/python

	@echo "To activate your new environment:  source ${envDir}/bin/activate"

# Re-install the pod5 environments to refresh / update the environment with changes
update:
	source ${envDir}/bin/activate \
	&& pip install -e .[dev] \
	&& pip install --editable ../../../src/python

	@echo "Updated python environment"
