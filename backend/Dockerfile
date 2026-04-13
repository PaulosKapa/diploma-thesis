#buld: docker build -t ecg-backend . 
#run: docker run -p 8000:8000 ecg-backend
FROM python:3.13-slim
#declare the workspace
WORKDIR /app
#get the necessary libraries
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
#copy the backend 
COPY . .
#open port 8000 (fastAPI works in the port)
EXPOSE 8000
#start the server
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
