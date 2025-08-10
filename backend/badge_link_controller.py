from config import Config
from database import db
from typing import Optional, Union
import jwt

config = Config("config.yaml")

class BadgeLinkController:
    DB_NAME = "badge_link"

    @staticmethod
    async def translate_uid_to_user(uid: str) -> Optional[int]:
        """
        Translate attendee UID to user ID.
        Return None if the attendee is not yet linked to with a badge.
        """
        result = await db[BadgeLinkController.DB_NAME].find_one({"uid": uid})
        return result["badge_user"] if result else None


    @staticmethod
    async def parse_badge_token(token: str) -> Optional[str]:
        """
        Retrieve UID and Name from badge token (JWT).
        Return None if the token is invalid.
        """
        user = jwt.decode(token, config.get("hitcon", {}).get("secret", ""), algorithms=["HS256"])

        if "scope" not in user or "badge" not in user["scope"]: return

        return user.get("sub"), user.get("nick")


    @staticmethod
    async def link_badge_with_attendee(uid: str, badge_user: int, name: str) -> tuple[Union[None, int], int]:
        """
        Link an attendee to a badge.
        Raise exception if the badge is already linked to another attendee.
        Return a tuple of (old_badge_user, new_badge_user).
        """
        result = await db["users"].find_one({"user": badge_user})  # Ensure the official badge
        if not result:
            raise ValueError(f"Non-official badge!")

        existing_badge_link = await db[BadgeLinkController.DB_NAME].find_one({"badge_user": badge_user})
        if existing_badge_link:
            raise ValueError(f"Badge {badge_user} is already linked to another user!")

        old_badge_user = await BadgeLinkController.translate_uid_to_user(uid)
        if old_badge_user is not None and old_badge_user != badge_user:
            await db[BadgeLinkController.DB_NAME].delete_one({"uid": uid, "badge_user": old_badge_user})

        # skip same badge duplicated linking
        if old_badge_user == badge_user:
            return None, badge_user

        await db[BadgeLinkController.DB_NAME].insert_one({"uid": uid, "badge_user": badge_user, "name": name})
        return old_badge_user, badge_user  # Return None for the previous badge user, and the new badge user ID
