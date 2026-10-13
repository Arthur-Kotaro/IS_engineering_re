import bcrypt
import hashlib
import secrets
import string

MAX_PASSWORD_LENGTH = 72


def truncate_password(password: str) -> str:
    pwd_bytes = password.encode('utf-8')
    if len(pwd_bytes) > MAX_PASSWORD_LENGTH:
        return pwd_bytes[:MAX_PASSWORD_LENGTH].decode('utf-8', errors='ignore')
    return password


def hash_password(password: str) -> str:
    truncated = truncate_password(password)
    salt = bcrypt.gensalt()
    return bcrypt.hashpw(truncated.encode('utf-8'), salt).decode('utf-8')


def verify_password(plain_password: str, hashed_password: str) -> bool:
    try:
        return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password.encode('utf-8'))
    except Exception:
        return False


def get_password_hash(password: str) -> str:
    return hash_password(password)


def hash_token(token: str) -> str:
    return hashlib.sha256(token.encode()).hexdigest()


def verify_token_hash(token: str, hashed_token: str) -> bool:
    return hash_token(token) == hashed_token


def generate_random_token(length: int = 32) -> str:
    return secrets.token_urlsafe(length)


def generate_reset_token() -> str:
    return secrets.token_urlsafe(32)


# --- Генерация временного пароля для нового пользователя ---

# Без похожих символов: 0/O, 1/l/I
_TEMP_PASSWORD_UPPER = "ABCDEFGHJKLMNPQRSTUVWXYZ"
_TEMP_PASSWORD_LOWER = "abcdefghijkmnpqrstuvwxyz"
_TEMP_PASSWORD_DIGITS = "23456789"
_TEMP_PASSWORD_SPECIAL = "!@#$%^&*"


def generate_temporary_password(length: int = 12) -> str:
    """
    Генерирует случайный пароль заданной длины.

    Гарантирует наличие:
    - хотя бы одной заглавной буквы,
    - хотя бы одной строчной буквы,
    - хотя бы одной цифры,
    - хотя бы одного специального символа.

    Символы 0/O, 1/l/I исключены — снижает вероятность ошибок при
    ручном вводе пароля пользователем.
    """
    if length < 8:
        raise ValueError("length must be at least 8")

    rng = secrets.SystemRandom()
    pwd = [
        rng.choice(_TEMP_PASSWORD_UPPER),
        rng.choice(_TEMP_PASSWORD_LOWER),
        rng.choice(_TEMP_PASSWORD_DIGITS),
        rng.choice(_TEMP_PASSWORD_SPECIAL),
    ]

    alphabet = (
        _TEMP_PASSWORD_UPPER
        + _TEMP_PASSWORD_LOWER
        + _TEMP_PASSWORD_DIGITS
        + _TEMP_PASSWORD_SPECIAL
    )
    while len(pwd) < length:
        pwd.append(rng.choice(alphabet))

    rng.shuffle(pwd)
    password = "".join(pwd)

    print(f"[hasher] generate_temporary_password: length={len(password)}")
    return password
