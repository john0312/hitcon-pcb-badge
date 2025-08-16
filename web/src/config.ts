export const env = {
    api: {
        getStations: '/api/stations',
        getScores: '/api/scores',
        period: 30000,
        timeout: 5000,
    },
    station: {
        maxStationID: 21,
    },
    score: {
        maximum: 1000,
        threshold: 100,
        sponsorIDs: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
        sponsorNames: ["DEVCORE", "Rakuten", "KlickKlack", "國家資通安全研究院", "Deloitte", "TWNIC", "CHT Security", "104人力銀行", "ISIP", "ASUS", "Findy", "CyCraft"],
    },
};