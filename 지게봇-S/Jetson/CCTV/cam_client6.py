import threading
import time
import re
import torch
import cv2
import socket

# YOLOv5 모델 로드
model = torch.hub.load('ultralytics/yolov5', 'custom', path='./box_model.pt')
#model.conf = 0.25

# 웹캠 연결
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("웹캠을 열 수 없습니다.")
    exit()

# 서버 연결 설정
HOST = "10.10.141.42"  # 서버의 IP 주소
PORT = 5000           # 서버와 통신할 포트
ADDR = (HOST, PORT)

# 소켓 생성 및 연결
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    s.connect(ADDR)
    print(f"서버에 성공적으로 연결되었습니다: {ADDR}")

    # 서버로 인증 메시지 전송 (접속 ID 사용)
    connection_id = "12"  # 접속할 때 사용할 ID
    password = "PASSWD"  # 비밀번호 (서버에서 설정된 비밀번호)
    auth_msg = f"[{connection_id}:{password}]"
    s.sendall(auth_msg.encode())  # 인증 메시지 전송
    s.sendall(b'\n')  # 인증 메시지 끝에 개행 문자 추가
    print(f"서버로 인증 메시지 전송: {auth_msg}")

except Exception as e:
    print(f"서버에 연결할 수 없습니다: {e}")
    exit()

# 명령에 사용할 ID 설정
command_id = "4"  # 명령 전송 시 사용할 ID

# 좌표 설정 (예시 좌표)
goal_coords = [(1.0, 2.0, 0.5, 0.5), (3.0, 4.0, 0.5, 0.5)]  # 이동할 좌표 리스트
current_goal_index = 0  # 현재 목표 지점 인덱스
detection_interval = 5  # 감지 간격 (초)
delay_after_move = 30  # 이동 후 대기 시간 (초)
delay_after_lift = 30  # liftUp 후 대기 시간 (초)

# 마지막 감지 시간을 0으로 초기화
last_detection_time = 0
frame = None  # 프레임 저장 변수
frame_lock = threading.Lock()  # 프레임 잠금 객체

def sendingMsg(msg):
    """서버로 메시지 전송"""
    msg_with_terminator = msg + '\n'
    s.sendall(msg_with_terminator.encode())
    print(f"서버로 메시지 전송: {msg_with_terminator}")

def gettingMsg():
    """서버로부터 메시지 수신"""
    global rsplit
    global recvFlag
    while True:
        try:
            data = s.recv(1024)
            if not data:
                break
            rstr = data.decode("utf-8")
            rsplit = re.split('[\]|\[@]|\n', rstr)
            recvFlag = True
        except Exception as e:
            print(f"메시지 수신 중 오류 발생: {e}")
            break

# 메시지 수신 스레드 시작
recvFlag = False
rsplit = []
message_receiver_thread = threading.Thread(target=gettingMsg)
message_receiver_thread.start()

def detect_objects():
    global last_detection_time
    global frame  # 메인 스레드의 프레임 사용
    global current_goal_index  # 추가: global 키워드 사용

    while True:
        # 프레임 읽기
        ret, new_frame = cap.read()

        if not ret:
            print("프레임을 읽을 수 없습니다.")
            continue

        # 새로운 프레임을 공유 변수에 저장
        with frame_lock:
            frame = new_frame

        current_time = time.time()

        # 감지 실행
        if current_time - last_detection_time >= detection_interval:
            last_detection_time = current_time

            # YOLOv5 모델로 감지 실행
            results = model(new_frame)

            # 감지된 객체 정보 (라벨을 포함한 bbox 정보)
            boxes = results.pandas().xyxy[0]

            # 바운딩 박스 그리기
            annotated_frame = results.render()[0]

            # 'box' 클래스가 감지되었는지 확인
            if len(boxes) > 0 and any(boxes['name'] == 'box'):
                # 현재 목표 지점으로 GOAL 메시지 전송
                custom_x, custom_y, custom_w, custom_h = goal_coords[current_goal_index]
                goal_msg = f"[{command_id}]GOAL@{custom_x}@{custom_y}@{custom_w}@{custom_h}"
                sendingMsg(goal_msg)

                # 이동 후 30초 대기
                time.sleep(delay_after_move)

                # "liftup" 명령 전송
                liftup_msg = f"[{command_id}]liftUp"
                sendingMsg(liftup_msg)

                # "liftup" 명령 전송 후 30초 대기
                time.sleep(delay_after_lift)

                # 다음 목표 지점으로 이동
                current_goal_index = (current_goal_index + 1) % len(goal_coords)
                next_x, next_y, next_w, next_h = goal_coords[current_goal_index]
                next_goal_msg = f"[{command_id}]GOAL@{next_x}@{next_y}@{next_w}@{next_h}"
                sendingMsg(next_goal_msg)

                # 다음 목표 지점으로 이동 후 30초 대기
                time.sleep(delay_after_move)

                # "liftdown" 명령 전송
                liftdown_msg = f"[{command_id}]liftDown"
                sendingMsg(liftdown_msg)
            with frame_lock:
                frame = annotated_frame
        # 감지 간격 외에도 웹캠 영상을 계속 업데이트
        with frame_lock:
            if frame is None:  # 새로운 프레임이 없다면 프레임을 읽어옴
                ret, new_frame = cap.read()
                if ret:
                    frame = new_frame

# 감지 스레드 시작
detection_thread = threading.Thread(target=detect_objects)
detection_thread.start()

while True:
    # 웹캠 프레임을 화면에 표시
    with frame_lock:
        if frame is not None:
            cv2.imshow('Webcam Feed', frame)

    # 'q'를 누르면 종료
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# 웹캠 및 창 닫기
cap.release()
cv2.destroyAllWindows()
s.close()
