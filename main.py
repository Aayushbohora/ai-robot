import speech_recognition as sr
import requests

ESP_IP = "http://192.168.1.50"  # ESP IP

r = sr.Recognizer()
with sr.Microphone() as source:
    print("Speak now...")
    audio = r.listen(source)

try:
    text = r.recognize_google(audio)  # Convert speech to text
    print("You said:", text)

    # Send text to ESP32
    requests.post(ESP_IP + "/voice", data={"msg": text})

except Exception as e:
    print("Error:", e)
