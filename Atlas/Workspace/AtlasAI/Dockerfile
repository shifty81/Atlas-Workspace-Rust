FROM python:3.12-slim

WORKDIR /app
COPY . /app/

# No external dependencies yet — stdlib only
# RUN pip install --no-cache-dir -r requirements.txt

ENTRYPOINT ["python", "arbiter_cli.py"]
CMD ["--help"]
