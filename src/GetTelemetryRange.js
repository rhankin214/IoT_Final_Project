const { MongoClient } = require("mongodb");

module.exports = async function (context, req) {
    const uri = process.env.MONGO_URI;
    const client = new MongoClient(uri);

    try {
        await client.connect();
        const db = client.db("iot");
        const col = db.collection("telemetry");

        const from = req.query.from ? new Date(req.query.from) : new Date("1970-01-01");
        const to = req.query.to ? new Date(req.query.to) : new Date();

        const POINTS_PER_DAY = 100;

        const pipeline = [
            {
                $match: {
                    timestamp: { $gte: from, $lte: to }
                }
            },
            {
                // Add a YYYY-MM-DD string for grouping
                $addFields: {
                    day: {
                        $dateToString: { format: "%Y-%m-%d", date: "$timestamp" }
                    }
                }
            },
            {
                $group: {
                    _id: "$day",
                    points: { $push: "$$ROOT" }
                }
            },
            {
                $project: {
                    sampled: {
                        $cond: [
                            { $lte: [{ $size: "$points" }, POINTS_PER_DAY] },
                            "$points",
                            {
                                $map: {
                                    input: {
                                        $range: [
                                            0,
                                            { $size: "$points" },
                                            {
                                                $ceil: {
                                                    $divide: [
                                                        { $size: "$points" },
                                                        POINTS_PER_DAY
                                                    ]
                                                }
                                            }
                                        ]
                                    },
                                    as: "i",
                                    in: { $arrayElemAt: ["$points", "$$i"] }
                                }
                            }
                        ]
                    }
                }
            },
            { $unwind: "$sampled" },
            { $replaceRoot: { newRoot: "$sampled" } },
            { $sort: { timestamp: 1 } }
        ];

        const docs = await col.aggregate(pipeline).toArray();

        context.res = {
            status: 200,
            headers: {
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            },
            body: docs
        };

    } catch (err) {
        context.res = {
            status: 500,
            headers: { "Access-Control-Allow-Origin": "*" },
            body: err.toString()
        };
    } finally {
        await client.close();
    }
};
