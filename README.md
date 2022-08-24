# ip_camera_api
Tiny python fast-api application to receive IP address from my esp32cam module


## Installation
To install all required pip packages and make a virtual env just run make install:
```
make install
mkdir -p python_env; \
	python3 -m venv python_env; \
	. python_env/bin/activate; \
	python3 -m pip install --upgrade pip; \
	python3 -m pip install -r requirements.txt; \
	python3 -m pip install -r requirements-test.txt
```


## Testing 
Run the included tests like so:
```
make test
```

Also get test coverage report:
```
make coverage
```


## Start webserver in production mode
This is best way to run as it accepts external requests (not only on localhost)
```
make server
```

## Start server during dev mode with live reload:
```
make dev
```
