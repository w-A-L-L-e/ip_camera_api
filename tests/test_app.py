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

# TODO: enforce check on api_key + tests here


def test_camera_ip():
    with TestClient(app) as app_client:
        response = app_client.get("/camera_ip")
        content = response.json()
        assert response.status_code == 200
        assert content['ip'] == 'server started'
        assert content['updated_at'] is not None


def test_post_new_ip():
    with TestClient(app) as app_client:
        TEST_IP_SUBMIT = '192.168.0.111'
        test_data = {
            'api_key': 'some_api_key_here',
            'IP': TEST_IP_SUBMIT
        }
        response = app_client.post(
            "/camera_update",
            json=test_data
        )

        assert response.status_code == 200
        assert response.text == '"OK"'

        # now test that submittted ip is returned on next
        # get call
        response = app_client.get("/camera_ip")
        content = response.json()
        assert response.status_code == 200
        assert content['ip'] == TEST_IP_SUBMIT


def test_root_path():
    with TestClient(app) as app_client:
        response = app_client.get("/")
        content = response.text
        assert response.status_code == 200
        assert 'swagger-ui' in content
