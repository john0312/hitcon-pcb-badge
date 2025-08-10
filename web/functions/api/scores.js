export async function onRequestGet(context) {
    return await fetch(`${context.env.BACKEND}/api/scores`);
}
