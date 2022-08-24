#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import uvicorn

if __name__ == "__main__":
    uvicorn.run("app.server:app", host="0.0.0.0", port=3000, access_log=False)
