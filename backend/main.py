from fastapi import FastAPI
from routes import predict
#run without docker
#uvicorn main:app --host 0.0.0.0 --port 8000 --reload
#start a fastAPI server
app = FastAPI()

#start a router to get the endpoints from the predict.py
app.include_router(
    predict.router,
    prefix="/api/v1/heart", 
)

#index page
@app.get('/')
async def index():
    return {"Message": "Server is online"}
