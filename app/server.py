#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#  @Author: Walter Schreppers
#
#   app/server.py
#   Tiny fast-api server for receiving IP address from esp32 camera module
#

import json
from fastapi import FastAPI, Request
from fastapi.responses import RedirectResponse, JSONResponse
from datetime import datetime

app = FastAPI(
    title="IP Camera API",
    description="Keep track of Camera IP",
    version="1.0.0",
)


@app.post("/camera_update")
async def camera_update(request: Request):
    request_data = await request.body()
    data = json.loads(request_data)
    app.camera_ip = data['IP']
    app.updated_at = datetime.now().isoformat()

    print("received IP = ", app.camera_ip)
    return 'OK'


@app.get("/camera_ip")
def get_ip():
    return {
        'ip': app.camera_ip,
        'updated_at': app.updated_at
    }


@app.on_event("startup")
def startup_event():
    app.camera_ip = 'server started'
    app.updated_at = datetime.now().isoformat()


@app.on_event('shutdown')
def shutdown_event():
    app.camera_ip = 'server stopped'
    app.updated_at = datetime.now().isoformat()


@app.get("/", include_in_schema=False)
def read_root():
    """ make root path show the API swagger docs """
    return RedirectResponse("/docs")
