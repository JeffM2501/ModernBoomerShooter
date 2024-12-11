#include "utilities/ai_utils.h"

namespace AIUtils
{
    float GetRotationTo(const Vector3& position, const Vector3& facing, const Vector3& target)
    {
        Vector3 vecToTarget = target - position;

        float disanceToTarget = Vector3Length(vecToTarget);

        Vector3 normalizedToTarget = vecToTarget / disanceToTarget;

        float targetDot = Vector3DotProduct(normalizedToTarget, facing);

        float dotAngle = acosf(targetDot) * RAD2DEG;

        if (Vector3DotProduct(vecToTarget, facing) >= 0)
            dotAngle *= -1;

        return dotAngle;
    }

    constexpr float MinAngleCos = 0.99984769515639123915701155881391f;

    Vector3 MoveTo(const Vector3& position, Vector3& facing, const Vector3& target, float maxMove, float maxRotation)
    {
        Vector3 vecToTarget = target - position;

        float disanceToTarget = Vector3Length(vecToTarget);

        Vector3 normalizedToTarget = vecToTarget / disanceToTarget;

        float targetDot = Vector3DotProduct(normalizedToTarget, facing);

        Vector3 facingNormal = Vector3{ facing.y, -facing.x, 0 };

        if (targetDot >= cosf(maxRotation * DEG2RAD)/*MinAngleCos*/)
        {
            facing = normalizedToTarget;
        }
        else
        {
            float rotAngle = maxRotation;
            if (Vector3DotProduct(vecToTarget, facingNormal) >= 0)
                rotAngle *= -1;

            Vector2 rot = Vector2Rotate(Vector2{ facing.x, facing.y }, rotAngle * DEG2RAD);
            facing.x = rot.x;
            facing.y = rot.y;
        }

        if (disanceToTarget < maxMove)
        {
            return target - position;
        }
        else
        {
            return facing * maxMove;
        }
    }
}