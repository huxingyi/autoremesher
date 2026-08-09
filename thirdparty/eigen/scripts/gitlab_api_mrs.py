"""Downloads all MRs from GitLab matching specified criteria."""

import argparse
import datetime
import json
import os
import requests
import sys

EIGEN_PROJECT_ID = 15462818  # Taken from the gitlab project page.


def date(date_string: str):
    """Convert a date YY-MM-DD string to a datetime object."""
    try:
        return datetime.strptime(date_string, "%Y-%m-%d")
    except ValueError:
        msg = f"Not a valid date: '{date_string}'. Expected format is YYYY-MM-DD."
        raise argparse.ArgumentTypeError(msg)


def _get_api_query(
    gitlab_private_token: str, url: str, params: dict[str, str] | None = None
):
    next_page = "1"
    if not params:
        params = dict()
    params["per_page"] = "100"
    headers = {"PRIVATE-TOKEN": gitlab_private_token}
    out = []
    while next_page:
        params["page"] = next_page
        try:
            resp = requests.head(url=url, params=params, headers=headers)
            if resp.status_code != 200:
                print("Request failed: ", resp, file=sys.stderr)
                break

            next_next_page = resp.headers["x-next-page"]

            resp = requests.get(url=url, params=params, headers=headers)
            if resp.status_code != 200:
                # Try again.
                continue

            out.extend(resp.json())
            # Advance at the end, in case an exception occurs above so we can retry
            next_page = next_next_page
        except:
            # Keep same next_page
            continue
    return out


def get_merge_requests(
    gitlab_private_token: str,
    author_username: str | None = None,
    state: str | None = None,
    created_before: datetime.datetime | None = None,
    created_after: datetime.datetime | None = None,
    updated_after: datetime.datetime | None = None,
    updated_before: datetime.datetime | None = None,
    related_issues: bool = False,
    closes_issues: bool = False,
):
    """Return list of merge requests.

    Args:
      gitlab_token: GitLab API token.
      author_username: MR author username.
      state: MR state (merged, opened, closed, locked).
      created_after: datetime start of period.
      created_before: datetime end of period.
      updated_after: datetime start of period.
      updated_before: datetime end of period.

    Returns:
      List of merge requests.
    """
    url = (
        "https://gitlab.com/api/v4/projects/"
        + str(EIGEN_PROJECT_ID)
        + "/merge_requests"
    )
    params = dict()
    if author_username:
        params["author_username"] = author_username

    if state:
        params["state"] = state

    if created_before:
        params["created_before"] = created_before.isoformat()

    if created_after:
        params["created_after"] = created_after.isoformat()

    if updated_before:
        params["updated_before"] = updated_before.isoformat()

    if updated_after:
        params["updated_after"] = updated_after.isoformat()

    params["order_by"] = "created_at"
    params["sort"] = "asc"

    next_page = "1"
    params["per_page"] = "100"
    headers = {"PRIVATE-TOKEN": gitlab_private_token}

    mrs = _get_api_query(gitlab_private_token, url, params)

    if related_issues:
        for mr in mrs:
            mr["related_issues"] = _get_api_query(
                gitlab_private_token, f"{url}/{mr['iid']}/related_issues"
            )

    if closes_issues:
        for mr in mrs:
            mr["closes_issues"] = _get_api_query(
                gitlab_private_token, f"{url}/{mr['iid']}/closes_issues"
            )

    return mrs


def main(_):
    parser = argparse.ArgumentParser(
        description="A script to download all MRs from GitLab matching specified criteria."
    )
    parser.add_argument(
        "--gitlab_private_token",
        type=str,
        help="GitLab private API token.  Defaults to the GITLAB_PRIVATE_TOKEN environment variable if set.",
    )
    parser.add_argument("--author", type=str, help="The name of the author.")
    parser.add_argument(
        "--state",
        type=str,
        choices=["merged", "opened", "closed", "locked"],
        help="The state of the MR.",
    )
    parser.add_argument(
        "--created_before",
        type=date,
        help="The created-before date in YYYY-MM-DD format.",
    )
    parser.add_argument(
        "--created_after",
        type=date,
        help="The created-after date in YYYY-MM-DD format.",
    )
    parser.add_argument(
        "--updated_before",
        type=date,
        help="The updated-before date in YYYY-MM-DD format.",
    )
    parser.add_argument(
        "--updated_after",
        type=date,
        help="The updated-after date in YYYY-MM-DD format.",
    )
    parser.add_argument(
        "--related_issues", action="store_true", help="Query for related issues."
    )
    parser.add_argument(
        "--closes_issues",
        action="store_true",
        help="Query for issues closed by the MR.",
    )

    args = parser.parse_args()

    if not args.gitlab_private_token:
        args.gitlab_private_token = os.getenv("GITLAB_PRIVATE_TOKEN")
        if not args.gitlab_private_token:
            print("Could not determine GITLAB_PRIVATE_TOKEN.", file=sys.stderr)
            parser.print_usage()
            sys.exit(1)

    # Parse the arguments from the command line
    mrs = get_merge_requests(
        gitlab_private_token=args.gitlab_private_token,
        author_username=args.author,
        state=args.state,
        created_before=args.created_before,
        created_after=args.created_after,
        updated_before=args.updated_before,
        updated_after=args.updated_after,
        related_issues=args.related_issues,
        closes_issues=args.closes_issues,
    )

    mr_str = json.dumps(mrs, indent=2)
    print(mr_str)


if __name__ == "__main__":
    main(sys.argv)
