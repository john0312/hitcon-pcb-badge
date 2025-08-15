import asyncio
from datetime import datetime, timedelta, timezone
import pymongo
from fastapi import FastAPI, Request
from fastapi.responses import HTMLResponse
from jinja2 import Template
from redis.asyncio import Redis
import game_logic



app = FastAPI()


EPS = 0.01

async def get_station_score(gl: game_logic._GameLogic, last_n_minute: int = None):
    scores = {
        i: {"score": [], "time": []}
        for i in range(1, game_logic.const.STATION_COUNT + 1)
    }

    latest_time: datetime = (await gl.attack_history.find_one({}, sort=[("timestamp", -1)], projection={"timestamp": 1, "_id": 0}))["timestamp"]
    total_time = latest_time - gl.start_time
    step = game_logic.const.STATION_SCORE_CACHE_MIN_INTERVAL

    async def one_station(station_id):
        data = scores[station_id]
        if last_n_minute is None:
            r = range(0, total_time.seconds + step, step)
        else:
            r = range(total_time.seconds, total_time.seconds - last_n_minute * 60, -step)
        for delta in r:
            current_time = gl.start_time + timedelta(seconds=(delta + EPS))
            score = await gl.get_station_score(station_id=station_id, before=current_time)
            data["score"].append(score)
            data["time"].append(current_time.astimezone(timezone(timedelta(hours=8))).isoformat())

    await asyncio.gather(*(one_station(station_id) for station_id in scores.keys()))

    return scores


@app.get("/draw_station_score", response_class=HTMLResponse)
async def draw_station_score_page(request: Request, last_n_minute: int):
    if last_n_minute == -1:
        last_n_minute = None

    # local debug usage
    # game_logic.const.STATION_SCORE_CACHE_MIN_INTERVAL = 1
    # game_logic.const.STATION_SCORE_DECAY_INTERVAL = 1
    redis_client = Redis(host='localhost', port=6379)
    # await redis_client.flushall()  # Clear all keys in Redis for testing
    gl = game_logic._GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), redis_client=redis_client)
    await gl.debug_set_start_time()

    scores = await get_station_score(gl, last_n_minute=last_n_minute)
    template = Template("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Draw Station Score</title>
    <script src="https://cdn.plot.ly/plotly-3.1.0.min.js" charset="utf-8"></script>
</head>
<body>
    <div id="plot"></div>
    <script>
        const scores = {{ scores | tojson }};
        console.log(scores);
        const traces = Object.entries(scores).map(([stationId, data]) => ({
            x: data.time,
            y: data.score,
            mode: 'lines',
            name: `Station ${stationId}`
        }));
        const layout = {
            title: {text: 'Station Score Over Time'},
            xaxis: {title: {text: 'Time (seconds)'}},
            yaxis: {title: {text: 'Score'}},
            height: 800,
        };
        Plotly.newPlot('plot', traces, layout);
    </script>
</body>
</html>
    """)
    return template.render(scores=scores)


@app.get("/get_attack_score_history")
async def get_attack_score_history(request: Request, user_id: int):
    gl = game_logic._GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"))
    await gl.debug_set_start_time()

    ret = [
        {k: v for k, v in record.items() if k != '_id'}
        async for record in gl.get_station_attack_history(player_id=user_id)
    ]
    return ret


@app.get("/get_user_score_history")
async def get_user_score_history(request: Request, user_id: int):
    gl = game_logic._GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"))
    await gl.debug_set_start_time()

    ret = [
        {k: v for k, v in record.items() if k != '_id'}
        async for record in gl.get_game_history(player_id=user_id)
    ]
    return ret


