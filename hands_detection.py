import cv2
import mediapipe as mp
from enum import Enum
import socket # for socket 
import sys 
import math
import numpy as np

def socket_connection(data, conn, addr):
  
  try:
    print(f"Conexión establecida desde {addr}")
      #check if the data hasn't changed in 5 iterations
      # for i in range(5):
      #   if i == 0:
      #     prev_message = data
      #   else:
      #     if data == prev_message:
      #       continue
      #     else:
      #       prev_message = data
      #       break
      #manda el mensaje al servidor
    conn.send(data.encode())
  except:
    print('No se pudo mandar mensaje al cliente')

  # si solo vamos a mandar un mensaje, en realidad no necesitamos recibir pero este sería el código
    # message = s.recv(1024).decode()
    # prev_message = message
    # print(message)
    # print(prev_message)

class Direction(Enum):  
  UP = 1
  DOWN = 2
  LEFT = 3
  RIGHT = 4
  BACK = 5
  FORWARD = 6
  ESTABLE = 7

def get_direction(coordinate_x, coordinate_y, coordinate_z,image):
  if coordinate_x < image.shape[1] / 3:
    return Direction.RIGHT
  elif coordinate_x > 2 * image.shape[1] / 3:
    return Direction.LEFT
  elif coordinate_y < image.shape[0] / 3:
    return Direction.UP
  elif coordinate_y > 2 * image.shape[0] / 3:
    return Direction.DOWN
  elif coordinate_z < 80:
    return Direction.BACK
  elif coordinate_z > 100:
    return Direction.FORWARD
  
  else:
    return Direction.ESTABLE


def draw_landmarks(image):
  results = hands.process(image)  
  if results.multi_hand_landmarks:
    for hand_landmarks in results.multi_hand_landmarks:
      mp_drawing.draw_landmarks(
            image,
            hand_landmarks,
            mp_hands.HAND_CONNECTIONS,
            mp_drawing_styles.get_default_hand_landmarks_style(),
            mp_drawing_styles.get_default_hand_connections_style())
      return image

def get_coordinates(image):
  distanceCM_ = distance_to_camera(image)
  results = hands.process(image)
  if results.multi_hand_landmarks:
    for hand_landmarks in results.multi_hand_landmarks:
      landmark_middle = hand_landmarks.landmark[mp_hands.HandLandmark.MIDDLE_FINGER_MCP]
      coordinate_x= landmark_middle.x * image.shape[1]
      coordinate_y= landmark_middle.y * image.shape[0]
      direction_ = get_direction(coordinate_x, coordinate_y, distanceCM_, image)
      return direction_

def distance_to_camera(image):
  results = hands.process(image)
  if results.multi_hand_landmarks:
    for hand_landmarks in results.multi_hand_landmarks:
      finger_landmark = hand_landmarks.landmark[mp_hands.HandLandmark.INDEX_FINGER_MCP]
      pinky_landmark = hand_landmarks.landmark[mp_hands.HandLandmark.PINKY_MCP]
      x1 = finger_landmark.x * image.shape[1]
      y1 = finger_landmark.y * image.shape[0]
      x2 = pinky_landmark.x * image.shape[1]
      y2 = pinky_landmark.y * image.shape[0]
      distance = math.sqrt((y2-y1)**2 + (x2-x1)**2)
      return distance

mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_hands = mp.solutions.hands

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = '0.0.0.0'
port = 12345 
s.bind((host, port))
s.listen()
conn, addr = s.accept()

# For webcam input:
cap = cv2.VideoCapture(0)
with mp_hands.Hands(
    model_complexity=0,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5) as hands:
  while cap.isOpened():
    success, image = cap.read()
    if not success:
      print("Ignoring empty camera frame.")
      # If loading a video, use 'break' instead of 'continue'.
      continue

    # To improve performance, optionally mark the image as not writeable to pass by reference.
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    draw_landmarks(image)
    direction = get_coordinates(image)
    if direction:
      print(f'Direction: {direction.name}, {direction.value}') 
      data_send = str(direction.value)
      print(data_send)
      socket_connection(data_send, conn, addr)

    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
    # Flip the image horizontally for a selfie-view display.
    cv2.imshow('CONTROL XARM', cv2.flip(image, 1))
    if cv2.waitKey(5) & 0xFF == 27:
      break
cap.release()

