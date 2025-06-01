# Use a lightweight official Python image
FROM python:3.10-slim-buster

# Set the working directory in the container
WORKDIR /app

# Install dependencies (Flask and Gunicorn for production)
# Using --no-cache-dir is good practice to keep image size down
RUN pip install --no-cache-dir Flask gunicorn requests bs4

# Copy the application code into the container
COPY jlc_scraper.py .

# Expose the port the application listens on
# This is documentation, you'll still need to map ports when running
EXPOSE 3928

# Command to run the application using Gunicorn
# 'app:app' refers to the 'app' object within the 'app.py' file
# --bind specifies the host and port to listen on (0.0.0.0 makes it accessible from outside the container)
# --workers specifies the number of worker processes (adjust based on your needs and server resources)
CMD ["gunicorn", "--bind", "0.0.0.0:3928", "--workers", "4", "jlc_scraper:app"]
