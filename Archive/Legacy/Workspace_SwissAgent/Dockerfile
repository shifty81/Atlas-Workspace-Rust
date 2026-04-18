FROM python:3.12-slim

WORKDIR /app
COPY . /app/

# Install dependencies when available
# RUN pip install --no-cache-dir -r requirements.txt

ENTRYPOINT ["python", "cli.py"]
CMD ["--help"]
