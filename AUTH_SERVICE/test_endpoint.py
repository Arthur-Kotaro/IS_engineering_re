from fastapi import FastAPI, Request
import uvicorn

app = FastAPI()

@app.api_route("/{path:path}", methods=["GET", "POST", "PUT", "DELETE", "PATCH"])
async def catch_all(request: Request, path: str):
    print(f"=== REQUEST ===")
    print(f"Method: {request.method}")
    print(f"Path: {path}")
    print(f"Query params: {dict(request.query_params)}")
    print(f"Headers: {dict(request.headers)}")
    print(f"Body: {await request.body()}")
    return {"path": path, "method": request.method, "params": dict(request.query_params)}

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8999)
