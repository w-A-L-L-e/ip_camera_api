#  @Author: Walter Schreppers
#
#  tests/test_app.py
#
#   This tests the main fast-api calls provided
#

import pytest
import json
import glob

# from datetime import datetime, timedelta
from fastapi.testclient import TestClient
from app.server import app

# TODO: check api_key


def test_camera_ip():
    with TestClient(app) as app_client:
        response = app_client.get("/camera_ip")
        assert response.status_code == 200
        content = response.json()

        assert content['ip'] == 'server started'
        assert content['updated_at'] is not None


def test_post_new_ip():
    with TestClient(app) as app_client:
        test_data = {
            'api_key': 'some_api_key_here',
            'IP': '192.168.0.111'
        }
        response = app_client.post(
            "/camera_update",
            json=test_data
        )

        assert response.status_code == 200
        assert response.text == '"OK"'


def test_swagger():
    with TestClient(app) as app_client:
        response = app_client.get("/")
        content = response.text
        assert response.status_code == 200
        assert 'swagger-ui' in content
