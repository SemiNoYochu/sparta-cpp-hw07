#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Drone.generated.h"

struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class UBoxComponent;

UCLASS()
class SPARTACPP07_API ADrone : public APawn
{
    GENERATED_BODY()

public:
    ADrone();

protected:
    // -------------------------------------------------------
    // 컴포넌트
    // -------------------------------------------------------

    // 루트 충돌체 — 물리/충돌의 기준이 되는 박스
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
    UBoxComponent* BoxRootComponent;

    // 드론 본체 스켈레탈 메시
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
    USkeletalMeshComponent* BodyMeshComponent;

    // 날개 4개 — 소켓 이름으로 BodyMesh에 부착
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
    UStaticMeshComponent* WingForwardLeftMeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
    UStaticMeshComponent* WingForwardRightMeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
    UStaticMeshComponent* WingBackwardLeftMeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
    UStaticMeshComponent* WingBackwardRightMeshComponent;

    // 카메라 암 — Controller 회전을 따라 카메라 방향 결정
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
    USpringArmComponent* SpringArmComponent;

    // 실제 카메라
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
    UCameraComponent* CameraComponent;

    // -------------------------------------------------------
    // 이동 설정 (에디터에서 조정 가능)
    // -------------------------------------------------------

    // 수평/수직 이동 최대 속도 (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed;

    // 마우스 입력에 의한 회전 속도 (도/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationSpeed;

    // 인공 중력 가속도 (cm/s²) — Tick에서 매 프레임 직접 계산
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GravityAcceleration;

    // 지상 이동 속도 (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroundMoveSpeed;

    // 공중 이동 속도 배율 (0~1) — 지상 속도에 곱해서 공중 속도 결정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AirControlMultiplier;

    // -------------------------------------------------------
    // 런타임 상태 (에디터 노출 불필요)
    // -------------------------------------------------------

    // 현재 Z축 속도 — 중력 누적/초기화에 사용
    float VerticalVelocity;

    // 지면 접촉 여부 — CheckGrounded()가 매 프레임 갱신
    bool bIsGrounded;

    // -------------------------------------------------------
    // 입력 캐싱
    // 입력 함수(MoveAction/LookAction)는 이벤트 발생 시에만 호출되므로
    // 값을 멤버에 저장해두고 Tick에서 DeltaTime과 함께 처리
    // -------------------------------------------------------
    FVector MoveInput;  // X: 전후, Y: 좌우, Z: 상하
    FVector LookInput;  // X: Yaw, Y: Pitch, Z: Roll

    // -------------------------------------------------------
    // 입력 바인딩 함수
    // UFUNCTION() 필수 — 리플렉션 시스템에 등록되어야 BindAction이 동작
    // -------------------------------------------------------
    UFUNCTION()
    void MoveAction(const FInputActionValue& Value);
    UFUNCTION()
    void LookAction(const FInputActionValue& Value);

    // -------------------------------------------------------
    // 내부 처리 함수
    // -------------------------------------------------------

    // 박스 하단에서 아래로 짧은 LineTrace를 쏴서 지면 접촉 여부 반환
    // 상태를 변경하지 않으므로 const
    bool CheckGrounded() const;

    // 수직 속도(VerticalVelocity) 갱신
    // 수직 입력 중이면 중력 누적 없음, 지상이면 0 유지, 공중이면 중력 누적
    void UpdateVerticalVelocity(float DeltaTime);

    // 수평 이동 (전후/좌우) 처리
    // 지상/공중 상태에 따라 다른 속도 적용
    void ApplyHorizontalMovement(float DeltaTime);

    // 수직 이동 (상하) 처리
    // 하강: SweepSingleByChannel로 충돌 감지 후 안전한 위치까지만 이동
    // 상승: AddActorWorldOffset으로 천장 충돌만 감지
    void ApplyVerticalMovement(float DeltaTime);

    // 회전 처리 (Yaw/Pitch/Roll)
    void ApplyRotation(float DeltaTime);

public:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};