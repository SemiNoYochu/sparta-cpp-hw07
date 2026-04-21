#include "SpartaCpp07/Public/Drone.h"

#include "EnhancedInputComponent.h"
#include "SpartaPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"

ADrone::ADrone()
{
    PrimaryActorTick.bCanEverTick = true;

    // -------------------------------------------------------
    // 루트 컴포넌트 설정
    // BoxComponent를 루트로 사용 — 충돌/이동의 기준
    // 물리 시뮬레이션은 직접 구현하므로 SimulatePhysics = false
    // -------------------------------------------------------
    BoxRootComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CapsuleRootComponent"));
    SetRootComponent(BoxRootComponent);
    BoxRootComponent->SetSimulatePhysics(false);

    // -------------------------------------------------------
    // 메시 컴포넌트 설정
    // 모두 BoxRootComponent 또는 BodyMesh의 소켓에 부착
    // -------------------------------------------------------
    BodyMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMeshComponent"));
    BodyMeshComponent->SetupAttachment(BoxRootComponent);
    BodyMeshComponent->SetSimulatePhysics(false);

    WingForwardLeftMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingForwardLeftMeshComponent"));
    WingForwardLeftMeshComponent->SetupAttachment(BodyMeshComponent, TEXT("ForwardLeft"));
    WingForwardLeftMeshComponent->SetSimulatePhysics(false);

    WingForwardRightMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingForwardRightComponent"));
    WingForwardRightMeshComponent->SetupAttachment(BodyMeshComponent, TEXT("ForwardRight"));
    WingForwardRightMeshComponent->SetSimulatePhysics(false);

    WingBackwardLeftMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingBackwardLeftMeshComponent"));
    WingBackwardLeftMeshComponent->SetupAttachment(BodyMeshComponent, TEXT("BackwardLeft"));
    WingBackwardLeftMeshComponent->SetSimulatePhysics(false);

    WingBackwardRightMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingBackwardRightMeshComponent"));
    WingBackwardRightMeshComponent->SetupAttachment(BodyMeshComponent, TEXT("BackwardRight"));
    WingBackwardRightMeshComponent->SetSimulatePhysics(false);

    // -------------------------------------------------------
    // 카메라 설정
    // SpringArm: bUsePawnControlRotation = false
    //   → Controller 회전이 아닌 액터 회전 자체로 카메라 방향 결정
    //   → 마우스 입력을 AddActorLocalRotation으로 직접 처리하므로
    //      Controller 회전에 의존하지 않음
    // -------------------------------------------------------
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(BoxRootComponent);
    SpringArmComponent->TargetArmLength = 300.0f;
    SpringArmComponent->bUsePawnControlRotation = false;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = false;

    // -------------------------------------------------------
    // 이동 설정 기본값
    // -------------------------------------------------------
    MoveSpeed            = 600.0f;
    RotationSpeed        = 100.0f;
    GravityAcceleration  = 980.0f;  // 실제 중력의 약 1배 (UE 단위: cm/s²)
    GroundMoveSpeed      = 600.0f;
    AirControlMultiplier = 0.4f;    // 공중에서 지상 속도의 40%

    // -------------------------------------------------------
    // 런타임 상태 초기화
    // -------------------------------------------------------
    VerticalVelocity = 0.0f;
    bIsGrounded      = false;
    MoveInput        = FVector::ZeroVector;
    LookInput        = FVector::ZeroVector;
}

void ADrone::BeginPlay()
{
    Super::BeginPlay();
}

void ADrone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 매 프레임 지면 접촉 여부 갱신
    bIsGrounded = CheckGrounded();

    // 처리 순서:
    // 1. 수직 속도 갱신 (중력 누적 or 초기화)
    // 2. 수평 이동 적용
    // 3. 수직 이동 적용 (충돌 감지 포함)
    // 4. 회전 적용
    UpdateVerticalVelocity(DeltaTime);
    ApplyHorizontalMovement(DeltaTime);
    ApplyVerticalMovement(DeltaTime);
    ApplyRotation(DeltaTime);

    // 입력값 리셋
    // 입력 함수는 이벤트 발생 시에만 호출되므로
    // 매 프레임 리셋하지 않으면 마지막 입력값이 계속 유지됨
    MoveInput = FVector::ZeroVector;
    LookInput = FVector::ZeroVector;
}

void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // UInputComponent*로 넘어오는 매개변수를 실제 타입으로 캐스팅
    // 엔진이 Enhanced Input 플러그인 활성화 시 실제로는 UEnhancedInputComponent를 생성하지만
    // 함수 시그니처가 부모 타입으로 고정되어 있어 Cast가 필요
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // IA 에셋 포인터에 접근하기 위해 PlayerController를 실제 타입으로 캐스팅
        if (ASpartaPlayerController* PC = Cast<ASpartaPlayerController>(GetController()))
        {
            if (PC->MoveInputAction)
            {
                EnhancedInput->BindAction(PC->MoveInputAction, ETriggerEvent::Triggered,
                    this, &ADrone::MoveAction);
            }
            if (PC->LookInputAction)
            {
                EnhancedInput->BindAction(PC->LookInputAction, ETriggerEvent::Triggered,
                    this, &ADrone::LookAction);
            }
        }
    }
}

void ADrone::MoveAction(const FInputActionValue& Value)
{
    // 입력값을 캐싱 — 실제 이동은 Tick에서 DeltaTime과 함께 처리
    // IA_Move Value Type: Axis3D (FVector)
    // X: 전후 (W/S), Y: 좌우 (A/D), Z: 상하 (E/Q)
    MoveInput = Value.Get<FVector>();
}

void ADrone::LookAction(const FInputActionValue& Value)
{
    // 입력값을 캐싱 — 실제 회전은 Tick에서 DeltaTime과 함께 처리
    // IA_Look Value Type: Axis3D (FVector)
    // X: Yaw (마우스 좌우), Y: Pitch (마우스 상하), Z: Roll (마우스 휠)
    LookInput = Value.Get<FVector>();
}

bool ADrone::CheckGrounded() const
{
    // 박스 하단에서 아래로 10cm LineTrace
    // 중심이 아닌 하단 기준으로 쏘는 이유:
    // 중심 기준이면 박스 절반 높이 안에 있는 지면을 감지하지 못함
    const float HalfHeight = BoxRootComponent->GetScaledBoxExtent().Z;
    const FVector Start    = GetActorLocation() - FVector(0.f, 0.f, HalfHeight);
    const FVector End      = Start - FVector(0.f, 0.f, 10.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);  // 자기 자신은 무시

    return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
}

void ADrone::UpdateVerticalVelocity(float DeltaTime)
{
    if (!FMath::IsNearlyZero(MoveInput.Z))
    {
        // 상하 입력 중에는 입력값이 직접 이동을 담당하므로
        // 중력 누적 없이 VerticalVelocity를 0으로 유지
        VerticalVelocity = 0.f;
    }
    else if (bIsGrounded && VerticalVelocity <= 0.f)
    {
        // 지면에 있고 낙하 중이 아니면 속도 초기화
        VerticalVelocity = 0.f;
    }
    else
    {
        // 공중 상태: 매 프레임 중력 가속도만큼 하강 속도 증가
        // Max로 최대 낙하 속도 제한 (터미널 벨로시티)
        VerticalVelocity -= GravityAcceleration * DeltaTime;
        VerticalVelocity  = FMath::Max(VerticalVelocity, -2000.f);
    }
}

void ADrone::ApplyHorizontalMovement(float DeltaTime)
{
    // 지상/공중 상태에 따라 이동 속도 결정
    const float Speed = bIsGrounded
        ? GroundMoveSpeed
        : GroundMoveSpeed * AirControlMultiplier;

    // 액터의 로컬 좌표계 기준으로 이동 방향 결정
    // GetActorForwardVector(): 액터가 현재 바라보는 방향
    // GetActorRightVector():   액터의 오른쪽 방향
    FVector Delta = FVector::ZeroVector;
    Delta += GetActorForwardVector() * MoveInput.X * Speed * DeltaTime;
    Delta += GetActorRightVector()   * MoveInput.Y * Speed * DeltaTime;
    Delta.Z = 0.f;  // 수평 이동에서 Z 제거 (수직은 ApplyVerticalMovement가 담당)

    if (!Delta.IsNearlyZero())
        AddActorWorldOffset(Delta, true);  // true = 충돌 스윕
}

void ADrone::ApplyVerticalMovement(float DeltaTime)
{
    const float HalfHeight = BoxRootComponent->GetScaledBoxExtent().Z;

    // 한 프레임 최대 이동 거리를 박스 절반 높이의 90%로 제한
    // 이 제한이 없으면 큰 속도에서 Sweep이 지면을 뚫을 수 있음
    const float MaxStep  = HalfHeight * 0.9f;
    float VerticalMove   = (MoveInput.Z * MoveSpeed + VerticalVelocity) * DeltaTime;
    VerticalMove         = FMath::Clamp(VerticalMove, -MaxStep, MaxStep);

    if (VerticalMove < 0.f)
    {
        // -------------------------------------------------------
        // 하강: SweepSingleByChannel로 이동 경로 전체를 사전 검사
        // 충돌이 있으면 HitResult.Time(0~1) 비율만큼만 이동
        // → 지면을 뚫지 않고 정확히 충돌 지점 직전에 멈춤
        // -------------------------------------------------------
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        const FVector StartPos  = GetActorLocation();
        const FVector EndPos    = StartPos + FVector(0.f, 0.f, VerticalMove);
        const FCollisionShape Box = FCollisionShape::MakeBox(
            BoxRootComponent->GetScaledBoxExtent() * 0.99f  // 0.99: 경계면 오차 방지
        );

        if (GetWorld()->SweepSingleByChannel(Hit, StartPos, EndPos, GetActorQuat(), ECC_Visibility, Box, Params))
        {
            // 충돌 지점까지만 이동
            SetActorLocation(StartPos + FVector(0.f, 0.f, VerticalMove * Hit.Time), false);
            VerticalVelocity = 0.f;
        }
        else
        {
            SetActorLocation(EndPos, false);
        }
    }
    else if (VerticalMove > 0.f)
    {
        // -------------------------------------------------------
        // 상승: AddActorWorldOffset으로 천장 충돌만 감지
        // 상승 시작 지점이 이미 지면에 닿아있어 Sweep이 오작동하므로
        // AddActorWorldOffset(bSweep=true)로 위쪽 충돌만 처리
        // -------------------------------------------------------
        FHitResult Hit;
        AddActorWorldOffset(FVector(0.f, 0.f, VerticalMove), true, &Hit);
        if (Hit.bBlockingHit)
        {
            VerticalVelocity = 0.f;
        }
    }
}

void ADrone::ApplyRotation(float DeltaTime)
{
    // AddActorLocalRotation: 월드 기준이 아닌 액터 로컬 기준으로 회전
    // → 드론이 기울어진 상태에서도 직관적인 조작감 유지
    const FRotator Delta(
        LookInput.Y * RotationSpeed * DeltaTime,  // Pitch: 마우스 상하
        LookInput.X * RotationSpeed * DeltaTime,  // Yaw:   마우스 좌우
        LookInput.Z * RotationSpeed * DeltaTime   // Roll:  마우스 휠
    );

    AddActorLocalRotation(Delta, true);
}